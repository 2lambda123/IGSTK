// this file defines the igstkSandBoxTests for the test driver
// and all it expects is that you have a function called RegisterTests
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif
#include <iostream>
#include "itkTestMain.h" 


void RegisterTests()
{
  REGISTER_TEST(igstkEllipsoidObjectTest);
  REGISTER_TEST(igstkRenderWindowTest);
  REGISTER_TEST(igstkSceneTest);
  REGISTER_TEST(igstkView2DTest);
  REGISTER_TEST(igstkView3DTest);
}
