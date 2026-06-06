# cli-stopwatch
A simple *full-window* command line stopwatch with Split/Timestamp feature. Just start the program and it will start counting in the terminal. Press "P" to plot the split time, press ctrl+c to print time and exit.\
The window supports dynamic resizing, so you can fill free to move and resize it after launch. It's completely written in C.

---

## ✅Prerequisites
- You just need gcc. You most likely already have it, but just in case [here](https://gcc.gnu.org/) it is.

---

## 💾Download and use
This section is explained for **Linux**. It may differ if you use another OS.
- Download the stopwatch.c file from this repository and put it in a folder.
- Open the terminal in that folder and compile the program:
  ```bash
  cd ~/your-folder
  gcc stopwatch.c -o stopwatch
  ```
- Make the compiled program executable:
  ```bash
  chmod u+x stopwatch
  ```
- From now on, always run the program with
  ```bash
  ./stopwatch
  ```
- You can use the key "P" to print a partial split (timestamp) of the elapsed time in the terminal.
- You can always use ctrl+c to print the elapsed time and immediately exit.

*Note:* There are also a couple of customization settings such as color, that you can explore in the first lines of the source code.

---

## 📷️Screeshots of the running stopwatch
![IMG0](https://github.com/toccifrancesco/cli-stopwatch/blob/main/Screenshots/sw-0.png)
![IMG1](https://github.com/toccifrancesco/cli-stopwatch/blob/main/Screenshots/sw-1.png)
