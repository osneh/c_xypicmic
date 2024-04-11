# executable + DistanceThreshold + NumPixels + ListOfPixels
#./xypicmic.exe 100 9 103 35 6 15 68 28 63 41 63 38 63 35 68 26 12 37 4 39
#./xypicmic.exe 100 10 33 25 7 30 97 12 95 34 43 23 38 22 11 16 42 47 42 50 43 1
./xypicmic.exe 100 15 77 23 51 28 47 29 28 33 103 18 54 31 122 17 98 43 117 33 57 21 15 12 50 19 46 25 72 44 72 47 
#./xypicmic.exe 100 2 101 44 83 48
#./xypicmic.exe 100 2 89 21 87 23
#./xypicmic.exe 100 11 77 23 51 28 47 29 28 33 103 18 117 33 57 21 15 12 50 19 72 44 72 47
#./xypicmic.exe 100 13 77 23 51 28 47 29 28 33 103 18 117 33 57 21 15 12 50 19 72 44 72 47 122 17 54 13
##cat data_example_6.txt | xargs ./xypicmic.exe 
python plotter.py

