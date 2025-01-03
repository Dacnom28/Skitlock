[ Author: amelias. ]

   _____ __   _ __  __           __
  / ___// /__(_) /_/ /___  _____/ /__ 
  \__ \/ //_/ / __/ / __ \/ ___/ //_/ 
 ___/ / ,< / / /_/ / /_/ / /__/ ,< 
/____/_/|_/_/\__/_/\____/\___/_/|_| 

Skitlock is a simple Ransomware implementation for Linux.
version: 1.0
Next Version: 1.2 [ to be relaunched ]

Project Description:

Skitlock is a simple ransomware implementation designed specifically for the
Linux operating system. This project was created as part of an exploration and
experimentation to better understand how ransomware works in the Linux environment,
as well as to improve technical understanding of modern encryption techniques used in ransomware.
for now Skitt has 2 supporting modules such as: snapshot_cleaner.cpp  vm_detector.cpp 
The main file is named Skitt.cpp.

How to Compile:

before compiling you have to adjust the C2 Server URL where you host the C2 server,
you can adjust it in the Skitt.cpp file.
Download or copy the Skitlock source code to your working directory.
Open a terminal and navigate to the directory where you saved the source code.
Gunakan perintah berikut untuk mengkompilasi proyek:
[ g++ -o skitt Skitt.cpp vm_detector.cpp snapshot_cleaner.cpp -lssl -lcrypto -lcurl -pthread ]

Credit:

This project is fully developed by amelias..
If you use, adapt, or distribute this code,
please give appropriate credit to the original developer by crediting amelias.
as the main creator of the Skitlock project.


Disclaimer:

This project was created for learning and research purposes only.
Use of this code for illegal activities or to damage another person's system is strictly prohibited.
[ The developer is not responsible for the use of this code by other parties for unlawful purposes ]
Make sure you comply with the laws and regulations in your area before using or modifying this code.


   [ end ]
  [ and ]
 [ and ]
[ and ]


