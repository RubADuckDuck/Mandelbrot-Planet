#include "Renderer.h"
#include "Network/GameState.h"

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
}

void Renderer::DrawRespectTo(uint32_t objID, uint8_t ascendLevels, uint8_t descendDepth) {
    if (objID == 0) {
        log(LOG_WARNING, "ID Cannot be 0.");
    }

    uint32_t currID = objID;
    GameObject* currObj = gameState_->GetGameObject(currID);


    for (uint8_t i = 0; i < ascendLevels; i++) {
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
    n_frontier.resize(descendDepth + 1, 0);
    transformationMats.resize(descendDepth + 1, 0);

    // for internal iteration
    std::vector<uint32_t> currGrid;
    uint32_t currChildID = 0;
    GameObject* currChildObj = 0;
    glm::mat4 currGridTransformMat;

    uint8_t depth = 0;

    currObj = gameState_->GetGameObject(currID);

    // set position of root. might as well have to fix this.
    if (currObj->ptrNodeTransform_) {
        currObj->modelTransformMat_ = currObj->ptrNodeTransform_->GetTransformMatrix();
    }
    else {
        currObj->modelTransformMat_ = glm::mat4(1);
    }


    obj_stack.push(currObj);
    n_frontier[0] += 1; // add 1 element at depth 0 

    while (obj_stack.empty() != true) {
        // get current object
        currObj = obj_stack.top();
        obj_stack.pop();
        // delete number of elements at current depth
        n_frontier[depth] -= 1;
        // if current depth value zeros out, 
        // and current obj has no child, 
        // that is what causes depth to retract.

        // currentTransoformation
        transformationMats[depth] = currObj->modelTransformMat_;

        RidableObject* currRidable = dynamic_cast<RidableObject*>(currObj);

        if (currRidable == nullptr) {
            // no children here 
            continue;
        }
        else {
            if (depth < descendDepth) {
                // if we can go deeper
                currGrid = currRidable->GetGrid();
                depth += 1;

                for (uint8_t index = 0; index < currGrid.size(); index++) {
                    // get current child at index from grid
                    currChildID = currGrid[index];

                    if (currChildID == 0) {
                        // 0 -> nullptr
                    }
                    else {
                        // get childObj
                        currChildObj = gameState_->GetGameObject(currChildID);

                        currGridTransformMat = currRidable->GetGridTransformAt(index);

                        // parentTransformation * currGridTransform * ptrNodeTransformation -> current Transformation
                        currChildObj->modelTransformMat_ =
                            transformationMats[depth - 1]
                            * currGridTransformMat
                            * currChildObj->ptrNodeTransform_->GetTransformMatrix();

                        // add to frontier 
                        obj_stack.push(currChildObj);

                        // increase number in current depth
                        n_frontier[depth] += 1;
                    }
                }
            }
        }

        // set depth 
        for (uint8_t i = depth; -1 < i; i--) {
            if (n_frontier[i] != 0) {
                depth = i;
                break;
            }
            else {
                if (i == 0) {
                    if (!obj_stack.empty()) {
                        log(LOG_ERROR, "Stack Should be empty");
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
