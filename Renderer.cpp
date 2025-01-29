#include "Renderer.h"
#include "Network/GameState.h"
#include <string>
#include <sstream>
#include <set>

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


void Renderer::SetTransformationsForEachGameObject() {
    // 
} 

/* 
    Example Tree: 

    Before Action:
        A: B, C
        B: A', D 
        C: A' 
        D: B', E 
        E: D' 

    A: (A), (A -> B) , (A -> C), (A -> B -> D), (A -> B -> D -> E) 
    A': (A -> B -> A'), (A -> C -> A') 

    B: (B) ,(B -> D), (B -> D -> E) 
    B': (B -> A'), (B -> D -> B') 

    C: (C)
    C': (C -> A')

    D: (D), (D -> E) 
    D': (D -> B'), (D -> E -> D') 



    After Action:  
        A: B, C, D
        B: A' 
        D: E 

    A: (A), (A -> B) , (A -> C), (A -> D), (A -> D -> E) 
    A': (A -> B -> A'), (A -> C -> A'), (A -> D -> A')

    B: (B) 
    B': (B -> A')

    C: (C) 
    C': (C -> A')

    D: (D), (D -> E)
    D': (D -> A'), (D -> E -> D')

    how the transition between BeforeAction -> AfterAction is done. 

    main transition is 
    GameState: (A -> B -> D) -> (A -> D) 
    However, actions that step up through the hierarchy will also be shown as going deeper into the hierarchy. 
    Also, we will look at the gamestate and how they are rendered separately. 

    But first it has to predict that D rode A, constructing the transition below.

    Visualtransition:(A -> B -> D) -> (A -> B -> A' -> D)  

    To Acheive successful interpolation it is nessesary to keep (B -> D) -> (B -> A' -> D)  

    This is how the visual transition will look like. 
    The interpolated transition will be applied to others as follows  

    While the interpolation is happening transition of D will be overriden as follows. 
    Each arrow between B and D is substituted with '-(i)->' which will be the result of the interpolation.

    A: (A), (A -> B) , (A -> C), (A -> B -(i)-> D), (A -> B -(i)-> D -> E) 
    A': (A -> B -> A'), (A -> C -> A') 

    B: (B) ,(B -(i)-> D), (B -(i)-> D -> E) 
    B': (B -> A'), (B -(i)-> D -> B') 

    C: (C)
    C': (C -> A')

    D: (D), (D -> E) 
    D': (D -> B'), (D -> E -> D')  

    Inorder for above to be applied the state Before action has to be captured.
    Now, what about cases where another movement is requested while another is already moving?  

    GameState: (A -> D -> E) -> (A -> E) 

    Visualtransition:(A -> D -> E) -> (A -> D -> A' -> E) 
    
    Seems like E is moving out from D. 
    Doens't he know it is rude to interupt?.. 
    Each arrow between D and E is substituted with '-(j)->' which will be the result of the interpolation.

    A: (A), (A -> B) , (A -> C), (A -> B -(i)-> D), (A -> B -(i)-> D -(j)-> E)
    A': (A -> B -> A'), (A -> C -> A')

    B: (B) ,(B -(i)-> D), (B -(i)-> D -(j)-> E)
    B': (B -> A'), (B -(i)-> D -> B')

    C: (C)
    C': (C -> A')

    D: (D), (D -(j)-> E)
    D': (D -> B'), (D -> E -> D')
    
    When interpolation of (A -> B -> D) -> (A -> D) is finished, 
    (A -> B -(i)-> D) will be mapped to (A -> D)
    those which only contain (B -(i)-> D) but start with B will be erased. 
    (D -> B') will be mapped to (D -> A')

    A: (A), (A -> B) , (A -> C), (A -> D), (A -> D -(j)-> E)
    A': (A -> B -> A'), (A -> C -> A')

    B: (B)
    B': (B -> A')

    C: (C)
    C': (C -> A')

    D: (D), (D -(j)-> E)
    D': (D -> A'), (D -(j)-> E -> D')

    You might be asking "How is it okay to make B': (B -(i)-> D ~) disappear?". 
    Remember What we were interpolating? It's this (A -> B -> D) -> (A -> B -> A' -> D). 
    Now that the interpolation is complete, (B -(i)-> D ~) = (B -> A' -> D ~), but that will be drawn though B: (B -> A') A: (A -> D ~)
    Rather the problem is that (A -> D ~) has been emtpy until this point. Let's make somthing sparkle there. This is enough.
*/
void Renderer::SetTransformationChainForEachGameObject() { 
    


}

void Renderer::DrawDepth(uint32_t objID, uint8_t descendDepth) {
    // debug 
    bool doLog = false;
    std::stringstream ss;

    ptrCamera_->Update();

    if (objID == 0) {
        log(LOG_WARNING, "ID Cannot be 0.");
    }

    uint32_t currID = objID;
    GameObject* currObj = gameState_->GetGameObject(currID);

    float itpValue = 0;

    std::stack<GameObject*> obj_stack;
    std::stack<GameObject*> transform_matrix_stack; 
    std::set<uint32_t> visitedGameObjectIDs;

    std::vector<uint8_t> n_frontier; // number of objects waiting to be processed at each depth  
    std::vector<glm::mat4> transformationMats; // transformations of the parents ex) if A-B-C is current lineage then this list contains transformation that correspond to -B, -C 
    std::vector<uint32_t> lineage; // keeps family tree 
    std::vector<GameObject*> ptrGameObjectsLineage; 
    n_frontier.resize(descendDepth + 1, 0);
    transformationMats.resize(descendDepth + 1, 0);
    lineage.resize(descendDepth + 1, 0);

    // for internal iteration
    std::vector<uint32_t> currGrid; 
    uint32_t currChildID = 0;
    GameObject* currChildObj = nullptr;
    glm::mat4 currGridTransformMat;


    // indecates how deep we are in. this stops us from going too deep 
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


    if (doLog) {
        ss << "Pushing root object (ID: " << currID << ") to stack";
        log(LOG_INFO, ss.str());
    }

    obj_stack.push(currObj);
    n_frontier[0] += 1; // add 1 element at currDepthIndex 0 
    lineage[0] = currID; // Root lineage


    // Actual iteration
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

        // set auxilary history values 
        lineage[currDepthIndex] = currObj->GetID();
        ptrGameObjectsLineage[currDepthIndex] = currObj; 

        //// read the top and pop
        //transformationMats[currDepthIndex] = currObj->modelTransformMats_.top();
        //currObj->modelTransformMats_.pop();

        // this->DrawMesh(currObj->meshID_, currObj->textureID_, transformationMats[currDepthIndex]);


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
                // increasing depth
                currDepthIndex += 1;

                // iterate thorugh objects on grid 
                for (uint8_t index = 0; index < currGrid.size(); index++) {
                    // get current child at index from grid
                    currChildID = currGrid[index];

                    if (currChildID == 0) {
                        // empty spot
                        if (doLog) {
                            log(LOG_DEBUG, "Grid element is empty (Skipping)");
                        }
                    }
                    else {
                        // object exists

                        // check if this object has already been visited 
                        auto it = visitedGameObjectIDs.find(currChildID); 

                        if (it != visitedGameObjectIDs.end()) {
                            // it has already been visited 
                            // to do: not implemented yet
                        }
                        else {
                            // it hasn't been visited 

                            // get childObj 
                            currChildObj = gameState_->GetGameObject(currChildID);

                            if (currChildObj->interpolationValue_ >= 1) {
                                // currently not interpolating  

                                // Get (currObj -> currChild) transformMat
                                currGridTransformMat = currRidable->GetGridTransformAt(index);

                                // Set currentTransform to previous 
                                currChildObj->prevGridTransform_ = currGridTransformMat;

                                // parentTransformation * currGridTransform * ptrNodeTransformation -> current Transformation
                                // to do  
                            }
                            else {
                                // do interpolate 
                                currChildObj->interpolationValue_ += 0.1;
                                itpValue = currChildObj->interpolationValue_;

                                currGridTransformMat = (1 - itpValue) * currChildObj->prevGridTransform_
                                    + itpValue * currGrizzzz
                            }

                            
                            currGridTransformMat = currRidable->GetGridTransformAt(index); 

                            t

                            // no interpolation ----------------------------------------------
                            
                            // iterate through gameobject lineage to add this gameObject to the render cashe 

                            GameObject* currParent; 

                            currChildObj->AddToRenderList(currChildID, currChildObj->ptrNodeTransform_->GetTransformMatrix());

                            for (int parentsIndex = currDepthIndex - 1; parentsIndex > -1; parentsIndex--) { // decreaseing order 
                                // get parent
                                currParent = ptrGameObjectsLineage[parentsIndex]; 
                                
                                // get matrix that maps from (currentParent -> ... -> currentGameObject) 
                                currObj->GetMatrixFromRenderList(parentsIndex); 

                                // compose (currentParent -> ... -> currentGameObject) * gridTransformation * currChild.nodetransform 
                                // to obtain transformation (currentParent -> ... -> currentGameObject -> currChildObject) 
                                // add this transformation to the currentParentObject
                                currParent->AddToRenderList(
                                    currChildID, 
                                    currParent->ptrNodeTransform_->GetTransformMatrix()
                                    * currGridTransformMat
                                    * currChildObj->ptrNodeTransform_->GetTransformMatrix()
                                );
                            }

                            // do interpolation ------------------------------------------------
                            // 
                            // iterate through gameobject lineage to add this gameObject to the render cashe 
                          

                            GameObject* currGrandParent;

                            currChildObj->AddToRenderList(currChildID, currChildObj->ptrNodeTransform_->GetTransformMatrix()); 

                            // Transformation at currDepthIndex - 1 will be dealt manually, or just don't render at ptrGameObjectsLineage[currDepthIndex - 1]

                            for (int grantParentsIndex = currDepthIndex - 2; grantParentsIndex > -1; grantParentsIndex--) { // decreaseing order 
                                // get parent
                                currParent = ptrGameObjectsLineage[grantParentsIndex];

                                // get matrix that maps from (currentParent -> ... -> currentGameObject) 
                                currObj->GetMatrixFromRenderList(grantParentsIndex);

                                // compose (currentParent -> ... -> currentGameObject) * gridTransformation * currChild.nodetransform 
                                // to obtain transformation (currentParent -> ... -> currentGameObject -> currChildObject) 
                                // add this transformation to the currentParentObject
                                currParent->AddToRenderList(
                                    currChildID,
                                    currParent->ptrNodeTransform_->GetTransformMatrix()
                                    * currGridTransformMat
                                    * currChildObj->ptrNodeTransform_->GetTransformMatrix()
                                );
                            }












                            if (doLog) {
                                ss.str("");
                                ss << "Pushing child object (ID: " << currChildID << ") to stack";
                                log(LOG_INFO, ss.str());
                            }


                            // add to frontier 
                            obj_stack.push(currChildObj);

                            transform_matrix_stack.push()

                            // increase number of object in current depth
                            n_frontier[currDepthIndex] += 1;
                        }
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

    // ascend
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
    float itpValue = 0;

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
                        // empty spot
                        if (doLog) {
                            log(LOG_DEBUG, "Grid element is empty (Skipping)");
                        }
                    }
                    else {
                        auto it = std::find(lineage.begin(), lineage.begin() + currDepthIndex, currChildID); 

                        if (it != lineage.begin() + currDepthIndex) {
                            // if we find currChildren already in the lineage
                            // to do: not implemented yet
                        }
                        else {
                            // if nothing match 
                                                    
                            // get childObj 
                            currChildObj = gameState_->GetGameObject(currChildID); 

                            if (currChildObj->interpolationValue_ >= 1) {
                                // currently not interpolating  

                                // Get (currObj -> currChild) transformMat
                                currGridTransformMat = currRidable->GetGridTransformAt(index);  

                                // Set currentTransform to previous 
                                currChildObj->prevGridTransform_ = currGridTransformMat; 

                                // parentTransformation * currGridTransform * ptrNodeTransformation -> current Transformation
                                // to do  
                            }
                            else {
                                // do interpolate 
                                currChildObj->interpolationValue_ += 0.1; 
                                itpValue = currChildObj->interpolationValue_; 

                                currGridTransformMat = (1 - itpValue) * currChildObj->prevGridTransform_
                                    + itpValue * currGrizzzz
                            }

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
