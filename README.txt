PA2 EXTRA PRACTICE DRAFTS -- SIMULATED DEBUGGING EXERCISES
These buggy drafts are intentionally created for learning now, not evidence of historical iterations.

Basic_Draft_BuggyAnchor.cpp
  ERROR: Aligns only shape bounding-box origin to next empty cell.
  MISSES: Other piece occupied cells that could cover the target cell.
  Example: basic_sample.in => No solution (incorrect).
  Correction: loop over every occupied anchor cell in each orientation.

Bonus_Draft_FirstSolution.cpp
  ERROR: Stops at first complete solution, despite objective minimizing used piece count.
  Example: bonus_sample.in => Used pieces: 3 (valid tiling but not optimal).
  Correction: maintain bestK, update bestBoard, explore other branches with safe lower bounds.

Basic_Correct_V4.cpp and Bonus_Correct_V5.cpp
  These are copies of existing correctly solving versions, as reported by user passing official OJ.

Compile:
  g++ -std=c++11 -O2 Basic_Draft_BuggyAnchor.cpp -o basic_bug
  g++ -std=c++11 -O2 Bonus_Draft_FirstSolution.cpp -o bonus_bug
Run:
  ./basic_bug < basic_sample.in
  ./bonus_bug < bonus_sample.in

Note: The bonus draft's bug is correctness/optimality, not necessarily syntactic.
