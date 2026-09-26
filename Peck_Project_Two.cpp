// Peck_Project_Two.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <cctype>

using namespace std;

/*	Using maps to temporarily store data for an unsorted csv file.
	After the csv is processed to completion, process all courses
	with a prereqCount of '0' before incrementing.
	*/



//Struct to hold each course data. Will be populated after the csv is parsed completely
struct Course {
	string courseID;
	string courseName;

	vector<string> coursePrereqs;

	int prereqCount = 0;
};

//Struct for a node in the BST
struct TreeNode {
	Course courseData;
	TreeNode* left = nullptr;
	TreeNode* right = nullptr;

	//Constructor
	TreeNode(Course c) : courseData(c) {

	}
};

//New struct for the BPlusNode
struct BPlusNode {
	bool isLeaf;				//Leaf node or guidepost
	vector<string> keys;		//Sorted keys (Course IDs

	//If Guidepost (Not Leaf)
	vector<BPlusNode*> children;		//Size = keys.size + 1

	//If leaf node
	vector<shared_ptr<Course>> data;

	BPlusNode* next;					//Link to the next leaf on the right

	//Constructor
	BPlusNode(bool leafStatus) {
		isLeaf = leafStatus;
		next = nullptr;
	}
};

class ABCU_BPlusTree {
private:
	BPlusNode* root;
	const int M = 4;				//Max order/children

	//Helper function to find the child index to navigate down to
	int findChildIndex(BPlusNode* node, const string& key) {
		auto it = upper_bound(node->keys.begin(), node->keys.end(), key);
		return distance(node->keys.begin(), it);
	}

	//Splits an overfilled child node
	void splitChildNode(BPlusNode* parent, int index, BPlusNode* child) {
		BPlusNode* sibling = new BPlusNode(child->isLeaf);

		// Midpoint calculation for M = 4
		int mid = child->keys.size() / 2;

		if (child->isLeaf) {
			// Leaf Split: Distribute keys and data payloads
			sibling->keys.assign(child->keys.begin() + mid, child->keys.end());
			sibling->data.assign(child->data.begin() + mid, child->data.end());

			child->keys.erase(child->keys.begin() + mid, child->keys.end());
			child->data.erase(child->data.begin() + mid, child->data.end());

			// Maintain the leaf linked-list pointers
			sibling->next = child->next;
			child->next = sibling;

			// Push up a copy of the sibling's first key
			parent->keys.insert(parent->keys.begin() + index, sibling->keys[0]);
		}
		else {
			// Internal Node Split: Distribute keys and child pointers
			sibling->keys.assign(child->keys.begin() + mid + 1, child->keys.end());
			sibling->children.assign(child->children.begin() + mid + 1, child->children.end());

			// The middle key goes completely up to the parent, out of the child
			string pushUpKey = child->keys[mid];

			child->keys.erase(child->keys.begin() + mid, child->keys.end());
			child->children.erase(child->children.begin() + mid + 1, child->children.end());

			parent->keys.insert(parent->keys.begin() + index, pushUpKey);
		}

		// Maintain the leaf linked-list pointers
		sibling->next = child->next;
		child->next = sibling;

		// Insert the new sibling pointer into the parent's children vector
		parent->children.insert(parent->children.begin() + index + 1, sibling);
	}

	//Recursive function to insert nodes into non-full paths
	void insertNonFull(BPlusNode* node, shared_ptr<Course> course) {
		if (node->isLeaf) {
			//Find the insertion point
			auto it = upper_bound(node->keys.begin(), node->keys.end(), course->courseID);
			int index = distance(node->keys.begin(), it);

			node->keys.insert(it, course->courseID);
			node->data.insert(node->data.begin() + index, course);
		}
		else {
			int index = findChildIndex(node, course->courseID);

			//If child is completely full, split the child first
			if (node->children[index]->keys.size() == M - 1) {
				splitChildNode(node, index, node->children[index]);

				//After the split, check which path to walk down
				if (course->courseID > node->keys[index]) {
					index++;
				}
			}
			insertNonFull(node->children[index], course);
		}
	}

public:
	ABCU_BPlusTree() : root(new BPlusNode(true)) {}

	//Print all courses in order
	void PrintInOrder() {
		BPlusNode* curr = root;

		//Move to the leftmost leaf node
		while (curr && curr->isLeaf) {
			curr = curr->children[0];
		}

		//Perform a check to see if any leaf nodes are present
		if (!curr || curr->keys.empty()) {
			cout << "No courses loaded in the system." << endl;
			return;
		}

		//Courses exist so move down in alphabetical order
		cout << "\n--- Alphabetical Course List ---" << endl;
		while (curr != nullptr) {
			for (size_t i = 0; i < curr->keys.size(); i++) {
				cout << curr->data[i]->courseID << " - " << curr->data[i]->courseName << " | Prereqs: ";

				//print any prereqs if any
				if (curr->data[i]->coursePrereqs.empty()) {
					cout << "None" << endl;
				}
				else {
					for (auto& prereqs : curr->data[i]->coursePrereqs) {
						cout << prereqs << " | ";
					}
					//End the line
					cout << endl;
				}
			}
			curr = curr->next; // Move to the next leaf node to the right
		}
	}

	// Public Insertion Interface
	void insert(shared_ptr<Course> course) {
		BPlusNode* r = root;

		// If root is full, the tree grows in height
		if (r->keys.size() == M - 1) {
			BPlusNode* newRoot = new BPlusNode(false);
			newRoot->children.push_back(r);
			splitChildNode(newRoot, 0, r);
			root = newRoot;
			insertNonFull(newRoot, course);
		}
		else {
			insertNonFull(r, course);
		}
	}

	// Exact Match Search (O(log N))
	shared_ptr<Course> search(const string& courseID) {
		BPlusNode* curr = root;

		// Traverse down the internal routing nodes
		while (!curr->isLeaf) {
			int idx = findChildIndex(curr, courseID);
			curr = curr->children[idx];
		}

		// We are at the leaf layer, look for the exact key match
		auto it = lower_bound(curr->keys.begin(), curr->keys.end(), courseID);
		if (it != curr->keys.end() && *it == courseID) {
			int idx = distance(curr->keys.begin(), it);
			return curr->data[idx];
		}
		return nullptr; // Not found
	}

	// Range Query
	void printRange(const string& startID, const string& endID) {
		BPlusNode* curr = root;
		while (!curr->isLeaf) {
			curr = curr->children[findChildIndex(curr, startID)];
		}

		bool keepGoing = true;
		while (curr && keepGoing) {
			for (size_t i = 0; i < curr->keys.size(); ++i) {
				if (curr->keys[i] >= startID && curr->keys[i] <= endID) {
					cout << curr->keys[i] << ": " << curr->data[i]->courseName << "\n";
				}
				if (curr->keys[i] > endID) {
					keepGoing = false;
					break;
				}
			}
			curr = curr->next; // Simply move right along the linked list
		}
	}
};

/*class ABCU_BST_Tree {

private:
	TreeNode* root = nullptr;

	//Private helper functiuon for recursive insertion
	TreeNode* insertRecursive(TreeNode* node, Course courseData) {
		if (node == nullptr) {
			return new TreeNode(courseData);
		}

		//Determine order in BST
		if (courseData.courseID < node->courseData.courseID) {
			node->left = insertRecursive(node->left, courseData);
		}
		else if (courseData.courseID > node->courseData.courseID){
			node->right = insertRecursive(node->right, courseData);
		}
		else {
			//Duplicates. Do nothing unless a bug appears
		}

		return node;
	}

	//Private recursive function for in-order order
	void inOrderRecursive(TreeNode* node) {
		if (node == nullptr) {
			return;
		}

		//Visit left side
		inOrderRecursive(node->left);

		//Print data from node
		std::cout << node->courseData.courseID << ", " << node->courseData.courseName << " | Prereqs: ";

		//print any prereqs if any
		if (node->courseData.coursePrereqs.empty()) {
			std::cout << "None" << std::endl;
		}
		else {
			for (auto& prereqs : node->courseData.coursePrereqs) {
				std::cout << prereqs << " | ";
			}
			//End the line
			std::cout << endl;
		}

		//Visit the right side
		inOrderRecursive(node->right);
	}

	TreeNode* searchRecursive(TreeNode* node, const std::string& courseID) {
		if (node == nullptr || node->courseData.courseID == courseID) {
			//Tree is empty or node was found
			return node;
		}

		//Smaller, traverse the left side
		if (courseID < node->courseData.courseID) {
			searchRecursive(node->left, courseID);
		}
		//Larger, traverse the right side
		else {
			searchRecursive(node->right, courseID);
		}
	}

public:
	//Public function to call for insertion
	void Insert(Course courseData) {
		root = insertRecursive(root, courseData);
	}

	//Public function to print the in-order traversal
	void PrintInOrder() {
		inOrderRecursive(root);
	}

	Course* search(const std::string& courseID) {
		TreeNode* foundNode = searchRecursive(this->root, courseID);

		if (foundNode == nullptr) {
			//Node not found
			std::cout << "Error: Course: " << courseID << " not found." << std::endl;
			return nullptr;
		}
		else {
			return &(foundNode->courseData);
		}
	}
};

*/

//Course ID with names
map<string, string> courseNames;

//Courses with prerequisites
map<string, vector<string>> prerequisiteList;

//Course IDs with prereqs count
map<string, int> prereqCount;

string ConvertToUpper(string lowerString) {

	//Store a temporary copy of string
	string tempString = lowerString;
	transform(tempString.begin(), tempString.end(), tempString.begin(), ::toupper);

	//return converted string
	return tempString;
}
void parseCSVFile(const string& fileName) {

	//Begin by opening the csv file
	ifstream file(fileName);

	//First check if the file is already open before attemtping to open
	if (!file.is_open()) {
		cerr << "Error opening file: " + fileName << endl;
		return;
	}

	//Create a variable to hold each line for processing
	string line;

	//Read the csv line-by-line
	while (getline(file, line)) {

		stringstream ss(line);
		string field;

		//Create an index to determine column position
		int column_index = 0;

		//initialize the ID field
		string courseID = "";

		//temporary vector storage
		vector<string> prereqCousesTemp;

		while (getline(ss, field, ','))
		{
			if (column_index == 0) {
				//store the courseID for later
				courseID = field;

				//Initialize Prereq count
				prereqCount[courseID] = 0;
			}
			else if (column_index == 1) {
				//Course name
				courseNames[courseID] = field;
			}
			else {
				//Prereq course found
				prereqCousesTemp.push_back(field);

				//Check for an empty line
				if (field != "") {
					//Add the prereq course to the map matching the course name
					prerequisiteList[courseID].push_back(field);

					//Increment the course prereq count to process based on count
					prereqCount[courseID]++;
				}
			}
			//Update the column index
			column_index++;
		}
		

		////Testing phase for insertion
		//std::cout << "Course ID: " + courseID <<
		//	" Course Name: " << courseNames[courseID];

		////Print any prereq if found
		//for (const std::string& prereqID : prereqCousesTemp) {
		//	std::cout << " | " << prereqID;
		//}

		////end with a new line
		//std::cout << endl;
	}
}

/*Once the CSV is parsed and data is entered into one of three map variables,
  the data can then be processed*/

void processCourseByDependency(
	const map<string, string>& IDNamesMap,
	const map<string, vector<string>>& IDPrereqsMap,
	map <string, int>& prereqCountMap,
	ABCU_BPlusTree& courseTree) {
	
	//create a queue to process courses based on count of prereqs
	queue<string> readyToProcess;

	//First process courses with '0' prerequisite courses
	for (const auto& pair : prereqCountMap) {
		if (pair.second == 0) {
			//Pushing the course ID
			readyToProcess.push(pair.first);
		}
	}

	/* In order to properly process courses with prerequisites, we have to first create a list of courses and order them in a way that makes
		sense, given their prerequisites. This is called a topological sort created by Kahn. This creates a list that tracks courses that can 
		be taken without prerequisites, or courses where the prerequisites have already been met. */

	//Main processing loop
	while (!readyToProcess.empty()) {

		//Load up the first course ID
		string currentCourseID = readyToProcess.front();

		//Remove from the queue while it's being processed
		readyToProcess.pop();

		// --- Safety Check ---
		//Get the course name (must exist in IDNamesMap if we are processing it)
		string courseName = IDNamesMap.at(currentCourseID);

		//Retrieve prerequisites.
		vector<string> currentPrereqs;

		// Check if the course ID exists in the Prereqs Map (handling the empty column case)
		if (IDPrereqsMap.count(currentCourseID) > 0) {
			// If it exists, use the actual data
			currentPrereqs = IDPrereqsMap.at(currentCourseID);
		}

		/*Changed how the data is added to the tree for the B+ Tree.*/

		auto newCourse = make_shared<Course>();
		newCourse->courseID = currentCourseID;
		newCourse->courseName = courseName;
		newCourse->coursePrereqs = currentPrereqs;
		newCourse->prereqCount = static_cast<int>(currentPrereqs.size());		//Forcing a smaller integer

		//Add to the B+ Tree via the shared ptr interface
		courseTree.insert(newCourse);

		//This checks if the course ID was one of the courses prerequisites
		for (auto& otherCoursePair : prereqCountMap) {
			string otherCourseID = otherCoursePair.first;

			// Check if the course ID exists in the main IDPrereqsMap before accessing
			if (IDPrereqsMap.count(otherCourseID) > 0) { // Check existence
				const vector<string>& prereqs = IDPrereqsMap.at(otherCourseID);

				//If the course was a prerequisites, it then removes the count remaining, if the count drops to 0, all prerequisites have been met
				if (find(prereqs.begin(), prereqs.end(), currentCourseID) != prereqs.end()) {
					otherCoursePair.second--;

					if (otherCoursePair.second == 0) {
						readyToProcess.push(otherCourseID);
					}
				}
			}
		}
	}
}


int main() {
	//Initialize the csv path name
	string csvPath = "ABCU_Advising_Program_Input.csv";

	int userChoice = 0;

	//Create instance of the BST
	ABCU_BPlusTree courseTree;

	while (userChoice != 9) {

		//Display menu
		cout << "Welcome to the Course Planner" << endl << endl;
		cout << "1.) Load Data Structures" << endl;
		cout << "2.) Print Course List" << endl;
		cout << "3.) Search Course" << endl;
		cout << "9.) Exit Program" << endl << endl;
		cout << "Please select an option: ";

		//Take in user input
		cin >> userChoice;

		switch (userChoice) {
		case 1:
			//Request the user to input the file name
			cout << "Please enter the file name: ";
			cin >> csvPath;

			//Begin parsing the CSV storing it locally til it can get processed
			parseCSVFile(csvPath);

			//Load courses from csv into data structure
			processCourseByDependency(courseNames, prerequisiteList, prereqCount, courseTree);

			break;
		case 2:
			//Print all courses
			courseTree.PrintInOrder();
			cout << endl << endl;
			break;
		case 3: {

			string searchString;
			//Search and print for a single course
			cout << "\nPlease enter a course ID you wish to search: ";
			cin >> searchString;

			//Change to use shared_ptr
			shared_ptr<Course> result = courseTree.search(ConvertToUpper(searchString));

			if (result != nullptr) {
				cout << endl << endl << "Found course: " << searchString << endl;
				cout << "Course ID: " << result->courseID << " | "
					<< "Course Name: " << result->courseName << " | "
					<< "Prerequisites: ";

				if (result->coursePrereqs.empty()) {
					cout << "None.";
				}
				else {
					for (size_t i = 0; i < result->coursePrereqs.size(); ++i) {
						cout << result->coursePrereqs[i];
						if (i < result->coursePrereqs.size() - 1) {
							cout << " | ";
						}
					}
				}
				break;
			}
			else {
				cout << "\nCourse ID " << searchString << " not found." << endl;
			}
		}
		}

		//Exit program
		cout << "Thank you for using the Course Planner. Have a good day.";

		return 0;

	}
}