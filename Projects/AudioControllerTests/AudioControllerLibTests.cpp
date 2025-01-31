#include "stdafx.h"

#include <queue>

#include <CppUnitTest.h>

#include "../AudioController/AudioControlInterface.h"
#include "Device.h"
#include "../AudioControllerLib/generate-uuid.h"
#include "CaseInsensitiveSubstr.h"



using namespace std::literals::string_literals;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ed::audio {
TEST_CLASS(AudioControllerLibTests) {
    // TEST_METHOD(CollectionEmpty)
    // {
    //     AudioControl ac;
    //     const std::unique_ptr<DeviceCollectionInterface> coll(ac.CreateDeviceCollection(L"Nothing At All"));
    //     Assert::IsTrue(coll->GetSize() == 0);
    // }

    TEST_METHOD(DeviceCtorTest)
    {
        const auto nameExpected = L"name01"s;
        const auto pnpIdExpected = generate_w_uuid();

        const Device dv(pnpIdExpected, nameExpected, DeviceFlowEnum::Capture, 100);

        Assert::AreEqual(nameExpected, dv.GetName());
        Assert::AreEqual(pnpIdExpected, dv.GetPnpId());
    }

    TEST_METHOD(FindSubstrCaseInsensitiveTest)
    {
        const auto substr01 = L"name01"s;
        const auto substr02 = L"nAmE01"s;
        const auto string01 = L"uu name01mm"s;

        Assert::IsTrue(FindSubstrCaseInsensitive(string01, substr01));
        Assert::IsTrue(FindSubstrCaseInsensitive(string01, substr02));
        Assert::IsTrue(FindSubstrCaseInsensitive(string01, L""s));

        Assert::IsFalse(FindSubstrCaseInsensitive(string01, substr02 + L"2"));
    }
};
}
