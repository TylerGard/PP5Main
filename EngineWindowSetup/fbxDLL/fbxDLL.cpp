// fbxDLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "fbxDLL.h"

struct fbxJoint {
	FbxNode *node;
	int parent_index = -1;
};


//std::vector<fbxJoint> jointVector;





namespace fbxNS 
{
	FbxManager* lSdkManager;
	FbxScene* lScene;
	FbxNode* lRootNode;
	meshStruct *lMesh;

	/* Tab character ("\t") counter */
	int numTabs = 0;

	int fbxFunctions::returnInt() {
		return 1;
	}
	/**
	* Print the required number of tabs.
	*/
	void PrintTabs() {
		for (int i = 0; i < numTabs; i++)
			printf("\t");
	}

	/**
	* Return a string-based representation based on the attribute type.
	*/
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type) {
		switch (type) {
		case FbxNodeAttribute::eUnknown: return "unidentified";
		case FbxNodeAttribute::eNull: return "null";
		case FbxNodeAttribute::eMarker: return "marker";
		case FbxNodeAttribute::eSkeleton: return "skeleton";
		case FbxNodeAttribute::eMesh: return "mesh";
		case FbxNodeAttribute::eNurbs: return "nurbs";
		case FbxNodeAttribute::ePatch: return "patch";
		case FbxNodeAttribute::eCamera: return "camera";
		case FbxNodeAttribute::eCameraStereo: return "stereo";
		case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
		case FbxNodeAttribute::eLight: return "light";
		case FbxNodeAttribute::eOpticalReference: return "optical reference";
		case FbxNodeAttribute::eOpticalMarker: return "marker";
		case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
		case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
		case FbxNodeAttribute::eBoundary: return "boundary";
		case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
		case FbxNodeAttribute::eShape: return "shape";
		case FbxNodeAttribute::eLODGroup: return "lodgroup";
		case FbxNodeAttribute::eSubDiv: return "subdiv";
		default: return "unknown";
		}
	}

	/**
	* Print an attribute.
	*/
	void PrintAttribute(FbxNodeAttribute* pAttribute) {
		if (!pAttribute) return;

		FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
		FbxString attrName = pAttribute->GetName();
		PrintTabs();
		// Note: to retrieve the character array of a FbxString, use its Buffer() method.
		printf("<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
	}

	void loadVertices(FbxNode* pNode, meshCollection &RmeshCollection) {
		FbxMesh* meshData = pNode->GetMesh();
		int num = meshData->GetControlPointsCount();

		for (int i = 0; i < num; i++) {
			fbxsdk::FbxVector4 hold;
			vertex temp;
			hold = meshData->GetControlPointAt(i);

			temp.position[0] = hold.mData[0];
			temp.position[1] = hold.mData[1];
			temp.position[2] = hold.mData[2];
			temp.position[3] = hold.mData[3];
			RmeshCollection.verts.push_back(temp);
		}

		

		int polygonNum = meshData->GetPolygonCount();
		float * uvPointer = nullptr;
		const char * uvName = nullptr;
		FbxStringList uvNames;
		meshData->GetUVSetNames(uvNames);
		uvName = uvNames[0];
		for (int i = 0; i < polygonNum; i++) {
			for (int j = 0; j < 3; j++) {
				RmeshCollection.indices.push_back(meshData->GetPolygonVertex(i, j));
				int uvIndex = meshData->GetPolygonVertex(i, j);
				FbxVector2 currentUV;
			
				bool boolVal;
				meshData->GetPolygonVertexUV(i, j, uvName, currentUV, boolVal);
				RmeshCollection.verts[uvIndex].U = static_cast<float>(currentUV[0]);
				RmeshCollection.verts[uvIndex].V = static_cast<float>(currentUV[1]);
			}
		}
	}

	/**
	* Print a node, its attributes, and all its children recursively.
	*/
	void PrintNode(FbxNode* pNode, meshCollection & RmeshCollection) {
		PrintTabs();
		const char* nodeName = pNode->GetName();

		FbxString attribute = GetAttributeTypeName((*pNode).GetNodeAttribute()->GetAttributeType());

		FbxDouble3 translation = pNode->LclTranslation.Get();
		FbxDouble3 rotation = pNode->LclRotation.Get();
		FbxDouble3 scaling = pNode->LclScaling.Get();
		if (attribute == "mesh") {
			// Print the contents of the node.
			printf("<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>\n",
				nodeName,
				translation[0], translation[1], translation[2],
				rotation[0], rotation[1], rotation[2],
				scaling[0], scaling[1], scaling[2]
			);
			loadVertices(pNode, RmeshCollection);
			numTabs++;
		}

		// Print the node's attributes.
		for (int i = 0; i < pNode->GetNodeAttributeCount(); i++)
			PrintAttribute(pNode->GetNodeAttributeByIndex(i));

		// Recursively print the children.
		for (int j = 0; j < pNode->GetChildCount(); j++)
			PrintNode(pNode->GetChild(j), RmeshCollection);

		numTabs--;
		PrintTabs();
		printf("</node>\n");
	}

	FbxNode *printBone(FbxNode* pNode, std::vector<FbxNode*> &vector) {
		int yourmomthree = pNode->GetChildCount();
		for (int j = 0; j < pNode->GetChildCount(); j++)
		{
			if (pNode->GetChild(j) != nullptr)
			{
				vector.push_back(printBone(pNode->GetChild(j), vector));
			}
		}
		return pNode;
	}
	void startPrintBone(FbxNode* pNode, std::vector<fbxJoint> &vector) {

		std::vector<FbxNode*> nodeVector;

		nodeVector.push_back(printBone(pNode,nodeVector));

		for (int i = 0; i < nodeVector.size(); i++) {

			fbxJoint tempJoint;

			tempJoint.node = nodeVector[i];

			vector.push_back(tempJoint);
		}
	}
	/**
	* Main function - loads the hard-coded fbx file,
	* and prints its contents in an xml format to stdout.
	*/
	void fbxFunctions::ProcessStuff(meshCollection & theCollection, float scalevalue) {
		std::vector<fbxJoint> jointVector;
		int poseCount = lScene->GetPoseCount();
		FbxPose *posePointer;
		for (int i = 0; i < poseCount; i++)
		{
			if (lScene->GetPose(i)->IsBindPose()) {
				posePointer = lScene->GetPose(i);
				break;
			}
		}

		meshStruct tMesh;
		tMesh.indexCount = posePointer->GetNode(0)->GetMesh()->GetControlPointsCount();

		FbxNode *nodePointer;
		int yourmom = posePointer->GetCount();
		for (int i = 0; i < posePointer->GetCount(); i++) {
			if (posePointer->GetNode(i)->GetSkeleton()) {
				if (posePointer->GetNode(i)->GetSkeleton()->IsSkeletonRoot()) {
					nodePointer = posePointer->GetNode(i);
					break;
				}
			}
		}

		startPrintBone(nodePointer, jointVector);

		for (int i = 0; i < jointVector.size(); i++) 
		{
			FbxNode *parentNode = jointVector[i].node->GetParent();
			for (int j = 0; j < jointVector.size(); j++) {
				if (parentNode == jointVector[j].node) {
					jointVector[i].parent_index = j;
					break;
				}
			}
		}

		std::vector<fbxJoint> tempVector;
		//for (int i = 0; i < jointVector.size(); i++) {
		//	int tempIndex;
		//	
		//	for (int j = 0; j < jointVector.size(); j++) {
		//		tempIndex = jointVector[j].parent_index;
		//		if (tempIndex == i) {
		//			//tempVector.push_back(jointVector[i]);
		//			tempVector.push_back(jointVector[j]);

		//		}
		//	}
		//}
		tempVector.push_back(jointVector[0]);
		tempVector.push_back(jointVector[jointVector[0].parent_index]);
		for (int i = 0; i < jointVector.size(); i++)
		{
			int tempIndex = jointVector[i].parent_index;
			if (tempIndex > 0)
			{
				tempVector.push_back(jointVector[i]);
				tempVector.push_back(jointVector[tempIndex]);
				
			}
		}
		jointVector = tempVector;

		std::vector<xyzw> someXYZW;
		/*for (int i = 0; i < jointVector.size(); i++) {
			FbxMatrix jointMatrix = jointVector[i].node->EvaluateGlobalTransform();
			regJoint mainJoint;
			auto matrix = jointVector[i].node->EvaluateGlobalTransform(0);
			auto pos = matrix.GetT();

			mainJoint.jXYZW.x = (float)pos.mData[0];
			mainJoint.jXYZW.y = (float)pos.mData[1];
			mainJoint.jXYZW.z = (float)pos.mData[2];
			mainJoint.jXYZW.w = (float)pos.mData[3];

			mainJoint.jMatrix.e00 = (float)matrix.Get(0, 0);
			mainJoint.jMatrix.e01 = (float)matrix.Get(0, 1);
			mainJoint.jMatrix.e02 = (float)matrix.Get(0, 2);
			mainJoint.jMatrix.e03 = (float)matrix.Get(0, 3);
			mainJoint.jMatrix.e10 = (float)matrix.Get(1, 0);
			mainJoint.jMatrix.e11 = (float)matrix.Get(1, 1);
			mainJoint.jMatrix.e12 = (float)matrix.Get(1, 2);
			mainJoint.jMatrix.e13 = (float)matrix.Get(1, 3);
			mainJoint.jMatrix.e20 = (float)matrix.Get(2, 0);
			mainJoint.jMatrix.e21 = (float)matrix.Get(2, 1);
			mainJoint.jMatrix.e22 = (float)matrix.Get(2, 2);
			mainJoint.jMatrix.e23 = (float)matrix.Get(2, 3);
			mainJoint.jMatrix.e30 = (float)matrix.Get(3, 0);
			mainJoint.jMatrix.e31 = (float)matrix.Get(3, 1);
			mainJoint.jMatrix.e32 = (float)matrix.Get(3, 2);
			mainJoint.jMatrix.e33 = (float)matrix.Get(3, 3);

			tMesh.jointVec.push_back(mainJoint);

			lMesh = &tMesh;

			someXYZW.push_back(mainJoint.jXYZW);
			theCollection.boneVerticesX.push_back(mainJoint.jXYZW);
		}
		theCollection.boneVerticesX.push_back(someXYZW);*/
		

		// Starting to grab animation data

		FbxAnimStack * animStack = lScene->GetCurrentAnimationStack();
		FbxTimeSpan timeSpan = animStack->GetLocalTimeSpan();
		FbxTime dur = timeSpan.GetDuration();
		FbxLong frameCount = dur.GetFrameCount(FbxTime::EMode::eFrames24);



		for (int i = 1; i < frameCount; i++) {
			FbxTime keyTime;
			keyTime.SetFrame(i, dur.eFrames24);
			std::vector<xyzw> someOtherXYZW;
			for (int j = 0; j < jointVector.size(); j++) {
				//FbxMatrix jointMatrix = jointVector[j].node->EvaluateGlobalTransform(keyTime);
			
				regJoint mainJoint;
				auto matrix = jointVector[j].node->EvaluateGlobalTransform(keyTime);//jointVector[i].node->EvaluateGlobalTransform(0);
				auto pos = matrix.GetT();

				mainJoint.jXYZW.x = (float)pos.mData[0];
				mainJoint.jXYZW.y = (float)pos.mData[1];
				mainJoint.jXYZW.z = (float)pos.mData[2];
				mainJoint.jXYZW.w = (float)pos.mData[3];

				mainJoint.jMatrix.e00 = (float)matrix.Get(0, 0);
				mainJoint.jMatrix.e01 = (float)matrix.Get(0, 1);
				mainJoint.jMatrix.e02 = (float)matrix.Get(0, 2);
				mainJoint.jMatrix.e03 = (float)matrix.Get(0, 3);
				mainJoint.jMatrix.e10 = (float)matrix.Get(1, 0);
				mainJoint.jMatrix.e11 = (float)matrix.Get(1, 1);
				mainJoint.jMatrix.e12 = (float)matrix.Get(1, 2);
				mainJoint.jMatrix.e13 = (float)matrix.Get(1, 3);
				mainJoint.jMatrix.e20 = (float)matrix.Get(2, 0);
				mainJoint.jMatrix.e21 = (float)matrix.Get(2, 1);
				mainJoint.jMatrix.e22 = (float)matrix.Get(2, 2);
				mainJoint.jMatrix.e23 = (float)matrix.Get(2, 3);
				mainJoint.jMatrix.e30 = (float)matrix.Get(3, 0);
				mainJoint.jMatrix.e31 = (float)matrix.Get(3, 1);
				mainJoint.jMatrix.e32 = (float)matrix.Get(3, 2);
				mainJoint.jMatrix.e33 = (float)matrix.Get(3, 3);

				tMesh.jointVec.push_back(mainJoint);

				lMesh = &tMesh;

				someOtherXYZW.push_back(mainJoint.jXYZW);
				//theCollection.boneVerticesX.push_back(mainJoint.jXYZW);
			}
			theCollection.boneVerticesX.push_back(someOtherXYZW);
			

		}
		//jointVector.clear();
	}

	

	meshStruct* fbxFunctions::getMesh() {
		return lMesh;
	}

	void fbxFunctions::initializeFBX() {

		// Change the following filename to a suitable filename value.
		const char* lFilename = "BM.fbx";
		const char* lFilenameTwo = "Teddy_Idle.fbx";
		// Initialize the SDK manager. This object handles all our memory management.
		lSdkManager = FbxManager::Create();

		// Create the IO settings object.
		FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
		lSdkManager->SetIOSettings(ios);

		// Create an importer using the SDK manager.
		FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

		// Use the first argument as the filename for the importer.
		if (!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings())) {
			printf("Call to FbxImporter::Initialize() failed.\n");
			printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
			exit(-1);
		}

		// Create a new scene so that it can be populated by the imported file.
		lScene = FbxScene::Create(lSdkManager, "myScene");

		// Import the contents of the file into the scene.
		lImporter->Import(lScene);

		// The file is imported; so get rid of the importer.
		lImporter->Destroy();

		// Print the nodes of the scene and their attributes recursively.
		// Note that we are not printing the root node because it should
		// not contain any attributes.
		meshCollection mageCollection;
		lRootNode = lScene->GetRootNode();
		if (lRootNode) {
			for (int i = 0; i < lRootNode->GetChildCount(); i++)
				PrintNode(lRootNode->GetChild(i), mageCollection);
		}

		
		ProcessStuff(mageCollection, 1.0f);
		
		meshColVector.push_back(mageCollection);



	lImporter = FbxImporter::Create(lSdkManager, "");
	if (!lImporter->Initialize(lFilenameTwo, -1, lSdkManager->GetIOSettings())) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		exit(-1);
	}
	
	
	lScene = FbxScene::Create(lSdkManager, "mySceneTwo");
	// Import the contents of the file into the scene.
	lImporter->Import(lScene);
	
	// The file is imported; so get rid of the importer.
	lImporter->Destroy();
	
	// Print the nodes of the scene and their attributes recursively.
	// Note that we are not printing the root node because it should
	// not contain any attributes.
	meshCollection teddyCollection;
	lRootNode = lScene->GetRootNode();
	if (lRootNode) {
		for (int i = 0; i < lRootNode->GetChildCount(); i++)
			PrintNode(lRootNode->GetChild(i), teddyCollection);
	}
	
	
	ProcessStuff(teddyCollection, 0.2f);
	
	meshColVector.push_back(teddyCollection);
		// Destroy the SDK manager and all the other objects it was handling.
		lSdkManager->Destroy();
	}
}