Rocket Subsystem Simulation
	A multithreaded C-based simulation designed to manage rocket subsystems, integrating resource management, 
	event-driven design, and real-time terminal visualization.

Features
	Multithreading:
		Utilizes Pthreads to manage subsystem and manager threads for concurrent execution.
	Synchronization:
		Ensures thread-safe operations with semaphores to manage shared resources.
	Dynamic Resource Management:
		Implements scalable arrays for resources and subsystems without relying on realloc.
	Event-Driven Design:
		Processes subsystem events dynamically with a priority-based event queue.
	Real-Time Visualization:
		Displays subsystem statuses, resource levels, and event logs dynamically in the terminal.

Technologies
	Languages: C
	Libraries: Pthreads, Semaphores
	Tools: Makefile, Valgrind, ANSI Terminal Codes

System Architecture
Here’s a high-level overview of the system:
	Subsystems:
		Each subsystem simulates real-world behaviors such as resource production and consumption.
		Runs as a separate thread with distinct statuses (e.g., FAST, SLOW, STANDARD).
	Manager:
		Coordinates subsystem operations, monitors resources, and resolves critical events.
	Event Queue:
		A priority-based data structure that dynamically manages and processes subsystem events.

How to Build and Run
	Requirements:
		GCC 
		Make utility
		Linux 

Steps:
	Clone the Repository:
		git clone https://github.com/timleeeeee/rocketSimulationSystem.git
		cd rocketSimulationSystem
		
	Build the Project:
		make

	Run the Simulation:
		./program
	
	Clean the Build:
		make clean

Contributing
If you’d like to contribute:

	Fork the repository.
	Create a feature branch: git checkout -b feature-name.
	Commit your changes: git commit -m "Add new feature".
	Push to your branch: git push origin feature-name.
	Open a pull request.

Future Improvements
	Add a graphical interface for subsystem visualization.
	Expand simulation complexity with interdependent subsystems.
	Optimize performance for high-concurrency scenarios.

License
This project is licensed under the MIT License. See the LICENSE file for more information.

Contact
	Timothy Lee
	Email: lee.timo0308@gmail.com





