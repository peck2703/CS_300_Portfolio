•	What was the problem you were solving in the projects for this course?


The client was concerned about creating a program that took in a document structed as a csv file, containing college courses and any prerequisite courses
if there were any. The client, ABC University wanted the program to allow a user to search for a specific course and to print information, such as course
ID, course name, and any prerequisite course ID(s). Another factor they requested was to print all offered courses in alphabetical order when the given 
document would not guarantee such an order upon receipt.

•	How did you approach the problem? Consider why data structures are important to understand.

The first problem was how to sort an unsorted document without hard coding any values into the system. When the document gets read by the program, it is
impossible to sort it at the time, because the application does not know the next line until it processes the current one, so we use data structures to hold
this data before we process. Another concern was that before any course could be processed and then ordered, there must be a check in place to verify that all
prerequisite courses must already exist, else the document’s line throws an exception.

•	How did you overcome any roadblocks you encountered while going through the activities or project?

The biggest problem with this project was the process of verifying that all course’s prerequisites existed before creating the object and adding it to a second
non-temporary data structure. After looking up solutions online, it was determined that the best way to verify was to store each line (which consisted of a course
ID, a course name, and then anything beyond those columns, meant it was a prerequisite), as a series of map variables. The first map variable was to store the 
course ID and name, the second one was for the course ID and a vector variable containing all its course prerequisite IDs, and the third being for the course ID,
and a count for how large the vector for the prerequisite courses were.  Then an algorithm known as Kahn’s presented itself, using queues of courses within this
vector to process the courses with the fewest prerequisites first, and then process the rest when their prerequisites were already processed, using the third map
variable containing the count, decrementing when one prerequisite was already added.

•	How has your work on this project expanded your approach to designing software and developing programs?

This was my first real venture into advanced data structures, apart from maps and vectors, and I got really into how the binary search trees were so efficient 
(and among the easiest of the three topics discussed here) that I began to really appreciate the trees. I still see the value of using the vector data structure 
but more as a means of temporary storage as the speed to search such a data type is very inefficient. I still have an issue with the Big ‘O’ notation, but that 
has always been the case with analysis and will take some time to really improve.

•	How has your work on this project evolved the way you write programs that are maintainable, readable, and adaptable?

Overall, most of the code that I do write is often readable, and adaptable, so none of that really changed over the course of this project. Most of the code that
I do write is within the Unreal Engine and it uses a visual scripting system that I organize and structure like a painting from the renaissance, and applying that
to standard code, is just bread and butter at this point. When it comes to maintainable, I assume this refers to commenting it for future refactoring or additional
team members analyzing code for future builds, and I usually comment my code aggressively, even adding comments for simple variable declarations, whether temporary
or otherwise. I feel that too much is better than not enough, but finding the perfect amount of comment versus code is a skill that needs refinement.
