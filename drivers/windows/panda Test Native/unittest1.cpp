#include "stdafx.h"
#include "CppUnitTest.h"
#include "panda/panda.h"

#include <tchar.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace panda;

namespace pandaTestNative
{
	TEST_CLASS(DeviceDiscovery)
	{
	public:

		TEST_METHOD(ListDevices)
		{
			auto pandas_available = Panda::listAvailablePandas();
			Assert::IsTrue(pandas_available.size() > 0, _T("No pandas were found."));
			for (auto sn : pandas_available) {
				Assert::IsTrue(sn.size() == 24, _T("panda Serial Number not 24 characters long."));
			}
		}

		TEST_METHOD(OpenFirstDevice)
		{
			auto pandas_available = Panda::listAvailablePandas();
			Assert::IsTrue(pandas_available.size() > 0, _T("No pandas were found."));

			auto p1 = Panda::openPanda(pandas_available[0]);
			Assert::IsFalse(p1 == nullptr, _T("Could not open panda."));
		}

		TEST_METHOD(OpenDeviceNoName)
		{
			auto pandas_available = Panda::listAvailablePandas();
			Assert::IsTrue(pandas_available.size() > 0, _T("No pandas were found."));

			auto p1 = Panda::openPanda("");
			Assert::IsFalse(p1 == nullptr, _T("Could not open panda."));
			Assert::IsTrue(p1->get_usb_sn() == pandas_available[0], _T("Could not open panda."));
		}

		TEST_METHOD(OpenDeviceUnavailable)
		{
			auto p1 = Panda::openPanda("ZZZZZZZZZZZZZZZZZZZZZZZZ");
			Assert::IsTrue(p1 == nullptr, _T("Invalid sn still worked."));
		}

		TEST_METHOD(WillNotOpenAlreadyOpenedDevice)
		{
			auto pandas_available = Panda::listAvailablePandas();
			Assert::IsTrue(pandas_available.size() > 0, _T("No pandas were found."));

			auto p1 = Panda::openPanda(pandas_available[0]);
			Assert::IsFalse(p1 == nullptr, _T("Could not open panda."));

			auto p2 = Panda::openPanda(pandas_available[0]);
			Assert::IsTrue(p2 == nullptr, _T("Opened an already open panda."));
		}

		TEST_METHOD(OpenedDeviceNotListed)
		{
			auto pandas_available = Panda::listAvailablePandas();
			Assert::IsTrue(pandas_available.size() > 0, _T("No pandas were found."));

			auto p1 = Panda::openPanda(pandas_available[0]);
			Assert::IsFalse(p1 == nullptr, _T("Could not open panda."));

			auto pandas_available2 = Panda::listAvailablePandas();
			for (auto sn : pandas_available2) {
				Assert::IsFalse(p1->get_usb_sn() == sn, _T("Opened panda appears in list of available pandas."));
			}

		}
	};


	TEST_CLASS(CANOperations)
	{
	public:

		TEST_METHOD(CANEcho)
		{
			auto p0 = Panda::openPanda("");
			Assert::IsFalse(p0 == nullptr, _T("Could not open panda."));

			p0->set_safety_mode(SAFETY_ALLOUTPUT);
			p0->set_can_loopback(TRUE);
			p0->can_clear(PANDA_CAN_RX);

			uint32_t addr = 0xAA;
			bool is_29b = FALSE;
			uint8_t candata[8];

			for (auto canbus : { PANDA_CAN1, PANDA_CAN2, PANDA_CAN3 }) {
				uint8_t len = (rand() % 8) + 1;
				for (size_t i = 0; i < len; i++)
					candata[i] = rand() % 256;

				p0->can_send(addr, is_29b, candata, len, canbus);
				Sleep(10);

				auto can_msgs = p0->can_recv();

				Assert::AreEqual<size_t>(2, can_msgs.size(), _T("Received the wrong number of CAN messages."), LINE_INFO());

				for (auto msg : can_msgs) {
					Assert::IsTrue(msg.addr == addr, _T("Wrong addr."));
					Assert::IsTrue(msg.bus == canbus, _T("Wrong bus."));
					Assert::IsTrue(msg.len == len, _T("Wrong len."));
					Assert::AreEqual(memcmp(msg.dat, candata, msg.len), 0, _T("Received CAN data not equal"));
					for(int i = msg.len; i < 8; i++)
						Assert::IsTrue(msg.dat[i] == 0, _T("Received CAN data not trailed by 0s"));
				}

				Assert::IsTrue(can_msgs[0].is_receipt, _T("Didn't get receipt."));
				Assert::IsFalse(can_msgs[1].is_receipt, _T("Didn't get echo."));
			}
		}

		TEST_METHOD(CANClearClears)
		{
			auto p0 = Panda::openPanda("");
			Assert::IsFalse(p0 == nullptr, _T("Could not open panda."));

			p0->set_safety_mode(SAFETY_ALLOUTPUT);
			p0->set_can_loopback(TRUE);
			p0->can_clear(PANDA_CAN_RX);

			uint8_t candata[8] = {'0', '1', '2', '3', '4', '5', '6', '7'};
			p0->can_send(0xAA, FALSE, candata, 8, PANDA_CAN1);
			Sleep(100);

			p0->can_clear(PANDA_CAN_RX);

			auto can_msgs = p0->can_recv();
			printf("DERP GOD %d messages\n", can_msgs.size());
			Assert::IsTrue(can_msgs.size() == 0, _T("Received messages after a clear."));
		}
	};

	TEST_CLASS(SerialOperations)
	{
	public:

		TEST_METHOD(LINEcho)
		{
			auto p0 = Panda::openPanda("");
			Assert::IsFalse(p0 == nullptr, _T("Could not open panda."));

			p0->set_safety_mode(SAFETY_ALLOUTPUT);

			for (auto lin_port : { SERIAL_LIN1, SERIAL_LIN2 }) {
				p0->serial_clear(lin_port);

				for (int i = 0; i < 10; i++) {
					uint8_t len = (rand() % LIN_MSG_MAX_LEN) + 1;
					std::string lindata;
					lindata.reserve(len);

					for (size_t j = 0; j < len; j++)
						lindata += (const char)(rand() % 256);

					p0->serial_write(lin_port, lindata.c_str(), len);
					Sleep(10);

					auto retdata = p0->serial_read(lin_port);
					Assert::AreEqual(retdata, lindata);
				}
			}
		}
	};
}