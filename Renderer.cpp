#include "Renderer.h"
#include "Network/GameState.h"
#include <string>
#include <sstream>

std::string PrintLineage(const std::vector<uint32_t>& lineage, uint8_t depth) {
    std::stringstream ss;
    ss << "Lineage: ";
    for (size_t i = 0; i <= depth && i < lineage.size(); ++i) {
        if (lineage[i] != 0) {
            ss << "->" << lineage[i] ;
        }
    }
    return ss.str();
}

std::string Renderer::GetName() const {
    return "Renderer";
}

void Renderer::log(LogLevel level, std::string text) {
    LOG(level, GetName() + "::" + text);
}

Renderer::Renderer(GameState* gameState)
    :
    gameState_(gameState)
{
    log(LOG_INFO, "Initializing Renderer");

    std::map<uint32_t, const std::string> ID2String = {
        {0, "Error"}
    };

    id2MeshAndTexture = std::make_unique<Type2MeshAndTexture<uint32_t>>(ID2String); 

    // init camera
    ptrCamera_ = std::make_unique<CameraObject>();
}

void Renderer::DrawRespectTo(uint32_t objID, uint8_t ascendLevels, uint8_t descendDepth) {
    // debug 
    bool doLog = false; 
    std::stringstream ss;

    ptrCamera_->Update(); 

    if (objID == 0) {
        log(LOG_WARNING, "ID Cannot be 0.");
    }

    uint32_t currID = objID;
    GameObject* currObj = gameState_->GetGameObject(currID);


    for (uint8_t i = 0; i < ascendLevels; i++) {
        if (doLog) {
            ss << "Ascending to parent (Level: " << (i + 1) << ")";
            log(LOG_INFO, ss.str());
        }

        // go up one level 
        currID = currObj->GetParentID();

        if (currID == 0) {
            log(LOG_WARNING, "failed to reference parent. object has no parent.");
            break;
        }
        currObj = gameState_->GetGameObject(currID);
    }

    // start rendering heirarchy 
    std::stack<GameObject*> obj_stack;
    std::vector<uint8_t> n_frontier;
    std::vector<glm::mat4> transformationMats;
    std::vector<uint32_t> lineage;
    n_frontier.resize(descendDepth + 1, 0);
    transformationMats.resize(descendDepth + 1, 0);
    lineage.resize(descendDepth + 1, 0); 

    // for internal iteration
    std::vector<uint32_t> currGrid;
    uint32_t currChildID = 0;
    GameObject* currChildObj = 0;
    glm::mat4 currGridTransformMat;

    uint8_t currDepthIndex = 0;

    currObj = gameState_->GetGameObject(currID);

    // When we decide the position of the transform, it is pushed in to the stack
    // and as it is drawn, this value is popped. 
    // The reason for this implementation is that same objects are drawn multiple times, 
    // So there are cases where a single spot for saving the matrix is not enough to keep track of. 
    if (currObj->ptrNodeTransform_) {
        currObj->modelTransformMats_.push(currObj->ptrNodeTransform_->GetTransformMatrix());
    }
    else {
        currObj->modelTransformMats_.push(glm::mat4(1));
    }

    if(doLog) {
        ss << "Pushing root object (ID: " << currID << ") to stack";
        log(LOG_INFO, ss.str());
    }

    obj_stack.push(currObj);
    n_frontier[0] += 1; // add 1 element at currDepthIndex 0 
    lineage[0] = currID; // Root lineage

    while (obj_stack.empty() != true) {

        if (doLog) {
            log(LOG_INFO, "---------------------------------------------------------------------");
        }

        // get current object
        currObj = obj_stack.top();
        obj_stack.pop();

        if (doLog) {
            ss.str("");
            ss << "Processing object (ID: " << currObj->GetID() << ") at depth: " << std::to_string(currDepthIndex + 1);
            log(LOG_INFO, ss.str());

            log(LOG_INFO, "Drawing Object");
        }
        



        // delete number of elements at current depth
        n_frontier[currDepthIndex] -= 1;
        // if current depth value zeros out, 
        // and current obj has no child, 
        // that is what causes depth to retract.
        lineage[currDepthIndex] = currObj->GetID(); 
        // read the top and pop
        transformationMats[currDepthIndex] = currObj->modelTransformMats_.top(); 
        currObj->modelTransformMats_.pop(); 

        this->DrawMesh(currObj->meshID_, currObj->textureID_, transformationMats[currDepthIndex]);

        if (doLog) {
            log(LOG_INFO, PrintLineage(lineage, currDepthIndex));
        }

        RidableObject* currRidable = dynamic_cast<RidableObject*>(currObj);

        if (currRidable == nullptr) {
            if (doLog) {
                log(LOG_INFO, "Object is not a RidableObject, skipping children");
            }
            continue;
        }
        else {
            if (currDepthIndex < descendDepth) {
                if (doLog) {
                    ss.str("");
                    ss << "Going deeper (Depth: " << (currDepthIndex + 1) << "/" << std::to_string(descendDepth + 1) << ")";
                    log(LOG_INFO, ss.str());
                }

                // if we can go deeper
                currGrid = currRidable->GetGrid();
                currDepthIndex += 1;

                for (uint8_t index = 0; index < currGrid.size(); index++) {
                    // get current child at index from grid
                    currChildID = currGrid[index];

                    if (currChildID == 0) {
                        if (doLog) {
                            log(LOG_DEBUG, "Grid element is empty (Skipping)");
                        }
                    }
                    else {
                        // get childObj
                        currChildObj = gameState_->GetGameObject(currChildID);

                        currGridTransformMat = currRidable->GetGridTransformAt(index);

                        // parentTransformation * currGridTransform * ptrNodeTransformation -> current Transformation 
                        // push it to the stack 
                        currChildObj->modelTransformMats_.push(
                            transformationMats[currDepthIndex - 1]
                            * currGridTransformMat
                            * currChildObj->ptrNodeTransform_->GetTransformMatrix()
                        );

                        if (doLog) {
                            ss.str("");
                            ss << "Pushing child object (ID: " << currChildID << ") to stack";
                            log(LOG_INFO, ss.str());
                        }

                        // add to frontier 
                        obj_stack.push(currChildObj);

                        // increase number in current depth
                        n_frontier[currDepthIndex] += 1;
                    }
                }
            }
        }

        // set depth 
        for (uint8_t i = currDepthIndex; -1 < i; i--) {
            if (n_frontier[i] != 0) {
                currDepthIndex = i;
                break;
            }
            else {
                if (i == 0) {
                    if (!obj_stack.empty()) {
                        if (doLog) {
                            log(LOG_ERROR, "Stack Should be empty");
                        }
                    }
                    else {
                        if (doLog) {
                            log(LOG_DEBUG, "Stack is Empty");
                        }
                        break;
                    }
                }
            }
        }
        // the reason we don't only retract by -1 is that
        // a sudden finish from 
        // 010001 depth: 4
        // to 
        // 010000 depth: 1 could happen
        //
    }
}

void Renderer::DrawMesh(uint32_t meshID, uint32_t textureID, glm::mat4 transfromMatrix) {

    GeneralMesh* currMesh = id2MeshAndTexture->GetMesh(meshID);
    Texture* currTexture = id2MeshAndTexture->GetTexture(textureID);
    
    //{
    //    std::cout << "==============DrawInfo==============" << std::endl;
    //    std::cout << "MeshID: " << std::to_string(meshID) << std::endl;
    //    std::cout << "TextureID: " << std::to_string(textureID) << std::endl;
    //    std::cout << std::endl << transfromMatrix << std::endl;
    //    std::cout << "==============================================" << std::endl;
    //}

    currMesh->Render(*ptrCamera_, transfromMatrix, currTexture);

    return;
}
