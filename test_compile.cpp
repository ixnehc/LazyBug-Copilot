#include <atlimage.h>  
#include <string>  
void test() { CImage img; std::wstring path = L"test.png"; img.Save(CW2T(path.c_str())); }  
