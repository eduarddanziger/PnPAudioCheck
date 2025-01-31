#include "stdafx.h"

#include <stdlib.h>  

#include <queue>

#include <CppUnitTest.h>

#include "../AudioController/AudioControlInterface.h"
#include "DeviceCollection.h"


using namespace std::literals::string_literals;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ed::audio {
    TEST_CLASS(DeviceCollectionTests) {
#ifdef _DEBUG
private:
    _CrtMemState sOld;
    _CrtMemState sNew;
    _CrtMemState sDiff;
public:

        static int MyReportHook(int reportType, wchar_t* message, int* returnValue)
        {
            if (message) {
                Logger::WriteMessage(message);
            }
            return 0;
        
        }

        TEST_METHOD_INITIALIZE(MyInit)
        {
            _CrtMemCheckpoint(&sOld); //take a snapshot
        }

        TEST_METHOD_CLEANUP(CleanUp)
        {
            _CrtMemCheckpoint(&sNew);
            _CrtSetReportHookW2(_CRT_RPTHOOK_INSTALL, MyReportHook);

            if (_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
            {
                _CrtDbgReportW(_CRT_WARN, __WFILE__, __LINE__, nullptr, L"\n----------- _CrtMemDumpStatistics ---------\n");
                _CrtMemDumpStatistics(&sDiff);
                _CrtDbgReportW(_CRT_WARN, __WFILE__, __LINE__, nullptr, L"\n----------- _CrtMemDumpAllObjectsSince ---------\n");
                _CrtMemDumpAllObjectsSince(&sOld);
                _CrtDbgReportW(_CRT_WARN, __WFILE__, __LINE__, nullptr, L"\n----------- _CrtDumpMemoryLeaks ---------\n");
                _CrtDumpMemoryLeaks();
				Assert::IsTrue(false, L"Memory leak detected");
            }
            else
            {
                _CrtDbgReportW(_CRT_WARN, __WFILE__, __LINE__, nullptr, L"\n-----------No memory leaks found ---------\n");
            }

        }
            

        TEST_METHOD(CtorMemoryLeakTest)
        {
            DeviceCollection devColl(L""s, false);

            // char* s = new char[17];
            // strcpy_s(s, 17, "allocate_no_free");
            // char* ss = new char[14];
            // strcpy_s(ss, 14, "stackoverflow");
            // delete[] ss;

        }
        TEST_METHOD(ResetConmtentMemoryLeakTest)
        {
            DeviceCollection devColl(L""s, false);
			devColl.ResetContent();
        }
#endif
    };
}
