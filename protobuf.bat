protoc --cpp_out=. hello.proto
pause
rem FOR /R %f IN (*.cc) DO REN "%f" *.cpp
rem pause