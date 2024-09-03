#!/usr/bin/python
import psutil
import os
import subprocess
import curses

def get_processes():
    processes = []
    try:
        # Run the compiled C++ program and capture its output
        result = subprocess.run(['./process_info'], capture_output=True, text=True)
        for line in result.stdout.splitlines():
            parts = line.split(' ', 6)  # Adjust to split by 6 parts
            if len(parts) >= 7:
                pid, user, cpu, mem, vmem, uptime, command = parts
                processes.append({
                    'PID': pid,
                    'USER': user,
                    'CPU(%)': cpu,
                    'MEM(%)': mem,
                    'VMEM': vmem,
                    'UTime': uptime,
                    'COMMAND': command
                })
    except Exception as e:
        print(f"Error: {e}")
    return processes

def draw_bar_graph(stdscr, y, x, value, max_value, bar_width=20, color_filled=curses.COLOR_GREEN, color_empty=curses.COLOR_RED):
    # Define color pairs for bar graph
    curses.init_pair(4, color_filled, curses.COLOR_BLACK)   # Filled color
    curses.init_pair(5, color_empty, curses.COLOR_BLACK)     # Empty color
    curses.init_pair(3, curses.COLOR_WHITE, curses.COLOR_BLACK)    # Light blue text for number

    # Calculate the filled and empty portions
    filled_length = int(bar_width * (value / max_value))
    empty_length = bar_width - filled_length

    # Draw the left bracket
    stdscr.addstr(y, x, '[', curses.color_pair(3))

    # Draw the filled part of the bar
    stdscr.addstr(y, x + 1, '|' * filled_length, curses.color_pair(4))

    # Draw the empty part of the bar with red |
    stdscr.addstr(y, x + 1 + filled_length, '|' * empty_length, curses.color_pair(5))

    # Draw the right bracket and number after the bar
    stdscr.addstr(y, x + bar_width + 1, f'] {value:.2f}%', curses.color_pair(3))

def display_system_info(stdscr):
    curses.curs_set(0)
    stdscr.nodelay(1)
    stdscr.timeout(1000)

    # Initialize color pairs
    curses.start_color()
    curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK)  # Green text, black background for header
    curses.init_pair(2, curses.COLOR_WHITE, curses.COLOR_BLACK)   # White text, black background for processes
    curses.init_pair(3, curses.COLOR_CYAN, curses.COLOR_BLACK)    # Light blue text, black background for system info

    # Variables for scrolling
    start_line = 0
    max_lines = curses.LINES - 12  # Keep space for system info and header

    while True:
        stdscr.clear()

        # System Information
        cpu_usage = psutil.cpu_percent(interval=1)
        ram_usage = psutil.virtual_memory().percent
        disk_usage = psutil.disk_usage('/').percent

        current_user = os.getlogin()
        num_threads = psutil.cpu_count()
        total_tasks = len(psutil.pids())

        stdscr.addstr(0, 0, f"Current User: {current_user}", curses.color_pair(3))  # Light blue text
        stdscr.addstr(1, 0, f"Number of Threads: {num_threads}", curses.color_pair(3))  # Light blue text
        stdscr.addstr(2, 0, f"RAM Usage: {ram_usage:.2f}%", curses.color_pair(3))  # Light blue text
        stdscr.addstr(3, 0, f"Total Tasks: {total_tasks}", curses.color_pair(3))  # Light blue text
        stdscr.addstr(4, 0, f"Disk Usage: {disk_usage:.2f}%", curses.color_pair(3))  # Light blue text
        
        # Draw CPU usage bar graph with number
        stdscr.addstr(6, 0, "CPU Usage:", curses.color_pair(3))  # Light blue text
        draw_bar_graph(stdscr, 6, len("CPU Usage:") + 1, cpu_usage, 100)  # CPU usage out of 100%

        # Draw RAM usage bar graph with number
        stdscr.addstr(7, 0, "RAM Usage:", curses.color_pair(3))  # Light blue text
        draw_bar_graph(stdscr, 7, len("RAM Usage:") + 1, ram_usage, 100)  # RAM usage out of 100%

        # Draw Disk usage bar graph with number
        stdscr.addstr(8, 0, "Disk Usage:", curses.color_pair(3))  # Light blue text
        draw_bar_graph(stdscr, 8, len("Disk Usage:") + 1, disk_usage, 100)  # Disk usage out of 100%

        # Display Process Information
        processes = get_processes()
        stdscr.addstr(10, 0, "Processes:", curses.A_BOLD | curses.color_pair(1))  # Green text for header
        
        header = ["PID", "USER", "CPU(%)", "MEM(%)", "VMEM(GB)", "Time+", "COMMAND"]
        column_widths = [10, 10, 10, 10, 10, 10, curses.COLS - 60]  # Adjust column widths
        header_line = "".join(f"{h:<{column_widths[i]}}" for i, h in enumerate(header))
        
        # Display header with green text
        stdscr.addstr(11, 0, header_line, curses.color_pair(1))
        
        # Display processes with white text
        y = 12
        for proc in processes[start_line:start_line + max_lines]:
            command = proc['COMMAND']
            command_display = command[:curses.COLS - sum(column_widths[:6]) - 20]  # Limit command length
            proc_line = f"{proc['PID']:<{column_widths[0]}}{proc['USER']:<{column_widths[1]}}{proc['CPU(%)']:<{column_widths[2]}}{proc['MEM(%)']:<{column_widths[3]}}{proc['VMEM']:<{column_widths[4]}}{proc['UTime']:<{column_widths[5]}}{command_display:<{column_widths[6]}}"
            try:
                stdscr.addstr(y, 0, proc_line, curses.color_pair(2))  # White text
            except curses.error:
                pass  # Handle the error gracefully if the string cannot be displayed
            y += 1

        stdscr.refresh()

        # Handle user input for scrolling and exit
        key = stdscr.getch()
        if key == curses.KEY_DOWN:
            start_line = min(start_line + 1, len(processes) - max_lines)
        elif key == curses.KEY_UP:
            start_line = max(start_line - 1, 0)
        elif key in [ord('q'), ord('Q')]:
            break

if __name__ == "__main__":
    curses.wrapper(display_system_info)

