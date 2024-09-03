# **CTop - Console-Based Task Viewer**

**CTop** is a lightweight, console-based task manager built using a combination of C++ and Python. It provides real-time monitoring of system processes, CPU usage, RAM usage, and disk usage in a terminal environment. The interface is designed to be user-friendly, with support for scrolling and easy exit options.

## **Features**

- **Real-Time Process Monitoring**: Displays a list of active processes with details like PID, user, CPU usage, memory usage, and more.
- **System Information**: Provides current user, number of threads, total tasks, CPU, RAM, and disk usage.
- **Interactive Scrolling**: Scroll through the list of processes using arrow keys.
- **Bar Graphs**: Visual representation of CPU, RAM, and disk usage with a dynamic bar graph.
- **Easy Exit**: Quit the application by pressing `q` or `Q`.

## **Installation**

To install and run **CTop**, follow these steps:

### **Prerequisites**

- Linux-based operating system.
- C++ compiler (like g++).
- Python 3.x installed on your system.
- `psutil` library for Python (can be installed using pip).

### **Steps**

1. **Clone the repository**:
    ```bash
    git clone https://github.com/yourusername/ctop.git
    cd ctop
    ```

2. **Compile the C++ program**:
    ```bash
    g++ -o process_info process_info.cpp
    ```

3. **Run the Python script**:
    ```bash
    python3 ctop.py
    ```

## **Usage**

Once you have installed and compiled the necessary files, you can run **CTop** with the following command:

```bash
python3 ctop.py
```

### **Controls**

- **Arrow Keys**: Scroll up and down through the list of processes.
- **q or Q**: Quit the application.

## **Contributing**

We welcome contributions! If you'd like to contribute to the project, feel free to fork the repository and submit a pull request.

## **License**

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## **Contact**

If you have any questions, feel free to reach out to us at [thesomen123@gmail.com](mailto:thesomen123@gmail.com).

---

Feel free to adjust any part of this `README.md` to better fit your project details.
