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

	vector<std::string> coursePrereqs;

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

class ABCU_BST_Tree {

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

//Course ID with names
std::map<std::string, std::string> courseNames;

//Courses with prerequisites
std::map<std::string, std::vector<std::string>> prerequisiteList;

//Course IDs with prereqs count
std::map<std::string, int> prereqCount;

string ConvertToUpper(std::string lowerString) {

	//Store a temporary copy of string
	std::string tempString = lowerString;
	std::transform(tempString.begin(), tempString.end(), tempString.begin(), ::toupper);

	//return converted string
	return tempString;
}
void parseCSVFile(const std::string& fileName) {

	//Begin by opening the csv file
	std::ifstream file(fileName);

	//First check if the file is already open before attemtping to open
	if (!file.is_open()) {
		std::cerr << "Error opening file: " + fileName << std::endl;
		return;
	}

	//Create a variable to hold each line for processing
	std::string line;

	//Read the csv line-by-line
	while (std::getline(file, line)) {

		std::stringstream ss(line);
		std::string field;

		//Create an index to determine column position
		int column_index = 0;

		//initialize the ID field
		std::string courseID = "";

		//temporary vector storage
		vector<std::string> prereqCousesTemp;

		while (std::getline(ss, field, ','))
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
	const map<std::string, std::string>& IDNamesMap,
	const std::map<std::string, vector<string>>& IDPrereqsMap,
	std::map <std::string, int>& prereqCountMap,
	ABCU_BST_Tree& courseTree) {
	
	//create a queue to process courses based on count of prereqs
	std::queue<std::string> readyToProcess;

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
		std::string currentCourseID = readyToProcess.front();

		//Remove from the queue while it's being processed
		readyToProcess.pop();

		// --- Safety Check ---
		//Get the course name (must exist in IDNamesMap if we are processing it)
		std::string courseName = IDNamesMap.at(currentCourseID);

		//Retrieve prerequisites.
		std::vector<std::string> currentPrereqs;

		// Check if the course ID exists in the Prereqs Map (handling the empty column case)
		if (IDPrereqsMap.count(currentCourseID) > 0) {
			// If it exists, use the actual data
			currentPrereqs = IDPrereqsMap.at(currentCourseID);
		}

		//Create a course object
		Course newCourse = { currentCourseID, IDNamesMap.at(currentCourseID), currentPrereqs };

		//Add to the BST as a node
		courseTree.Insert(newCourse);

		//This checks if the course ID was one of the courses prerequisites
		for (auto& otherCoursePair : prereqCountMap) {
			std::string otherCourseID = otherCoursePair.first;

			// Check if the course ID exists in the main IDPrereqsMap before accessing
			if (IDPrereqsMap.count(otherCourseID) > 0) { // Check existence
				const std::vector<std::string>& prereqs = IDPrereqsMap.at(otherCourseID);

				//If the course was a prerequisites, it then removes the count remaining, if the count drops to 0, all prerequisites have been met
				if (std::find(prereqs.begin(), prereqs.end(), currentCourseID) != prereqs.end()) {
					otherCoursePair.second--;

					if (otherCoursePair.second == 0) {
						readyToProcess.push(otherCourseID);
					}
				}
			}
		}
	}
}


int main(){
	//Initialize the csv path name
	string csvPath = "ABCU_Advising_Program_Input.csv";

	int userChoice = 0;

	//Create instance of the BST
	ABCU_BST_Tree courseTree;

	while (userChoice != 9) {

		//Display menu
		std::cout << "Welcome to the Course Planner" << std::endl << std::endl;
		std::cout << "1.) Load Data Structures" << std::endl;
		std::cout << "2.) Print Course List" << std::endl;
		std::cout << "3.) Search Course" << std::endl;
		std::cout << "9.) Exit Program" << std::endl << std::endl;
		std::cout << "Please select an option: ";

		//Take in user input
		cin >> userChoice;

		switch (userChoice) {
		case 1:
			//Request the user to input the file name
			std::cout << "Please enter the file name: ";
			cin >> csvPath;
			
			//Begin parsing the CSV storing it locally til it can get processed
			parseCSVFile(csvPath);

			//Load courses from csv into data structure
			processCourseByDependency(courseNames, prerequisiteList, prereqCount, courseTree);

			break;
		case 2:
			//Print all courses
			courseTree.PrintInOrder();
			std::cout << std::endl << std::endl;
			break;
		case 3:
			std::string searchString;
			//Search and print for a single course
			std::cout << "\nPlease enter a course ID you wish to search: ";
			std::cin >> searchString;

			Course* result = courseTree.search(ConvertToUpper(searchString));

			if (result != nullptr) {
				std::cout << std::endl <<std::endl << "Found course: " << searchString << std::endl;
				std::cout << "Course ID: " << result->courseID << " | "
					<< "Course Name: " << result->courseName << " | "
					<< "Prerequisites: ";

				if (result->coursePrereqs.empty()) {
					std::cout << "None.";
				}
				else {
					for (auto& prereqs : result->coursePrereqs) {
						std::cout << prereqs << " | ";
					}
					//End the line
					std::cout << std::endl;
				}
			}
			break;
		}
	}

	//Exit program
	std::cout << "Thank you for using the Course Planner. Have a good day.";

	return 0;

}