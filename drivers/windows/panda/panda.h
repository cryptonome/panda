#pragma once

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the PANDA_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// PANDA_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef PANDA_EXPORTS
#define PANDA_API __declspec(dllexport)
#else
#define PANDA_API __declspec(dllimport)
#endif

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

#include <windows.h>
#include <winusb.h>

#if defined(UNICODE)
#define _tcout std::wcout
#define tstring std::wstring
#else
#define _tcout std::cout
#define tstring std::string
#endif

#define LIN_MSG_MAX_LEN 10

//template class __declspec(dllexport) std::basic_string<char>;

namespace panda {
	typedef enum _PANDA_SAFETY_MODE : uint16_t {
		SAFETY_NOOUTPUT = 0,
		SAFETY_HONDA = 1,
		SAFETY_ALLOUTPUT = 0x1337,
	} PANDA_SAFETY_MODE;

	typedef enum _PANDA_SERIAL_PORT : uint8_t {
		SERIAL_DEBUG = 0,
		SERIAL_ESP = 1,
		SERIAL_LIN1 = 2,
		SERIAL_LIN2 = 3,
	} PANDA_SERIAL_PORT;

	typedef enum _PANDA_SERIAL_PORT_PARITY : uint8_t {
		PANDA_PARITY_OFF = 0,
		PANDA_PARITY_EVEN = 1,
		PANDA_PARITY_ODD = 2,
	} PANDA_SERIAL_PORT_PARITY;

	typedef enum _PANDA_CAN_PORT : uint8_t {
		PANDA_CAN1 = 0,
		PANDA_CAN2 = 1,
		PANDA_CAN3 = 2,
		PANDA_CAN_UNK = 0xFF,
	} PANDA_CAN_PORT;

	typedef enum _PANDA_CAN_PORT_CLEAR : uint16_t {
		PANDA_CAN1_TX = 0,
		PANDA_CAN2_TX = 1,
		PANDA_CAN3_TX = 2,
		PANDA_CAN_RX = 0xFFFF,
	} PANDA_CAN_PORT_CLEAR;

	typedef enum _PANDA_GMLAN_HOST_PORT : uint8_t {
		PANDA_GMLAN_CLEAR = 0,
		PANDA_GMLAN_CAN2 = 1,
		PANDA_GMLAN_CAN3 = 2,
	} PANDA_GMLAN_HOST_PORT;

	#pragma pack(1)
	typedef struct _PANDA_HEALTH {
		uint32_t voltage;
		uint32_t current;
		uint8_t started;
		uint8_t controls_allowed;
		uint8_t gas_interceptor_detected;
		uint8_t started_signal_detected;
		uint8_t started_alt;
	} PANDA_HEALTH, *PPANDA_HEALTH;

	typedef struct _PANDA_CAN_MSG {
		uint32_t addr;
		uint16_t recv_time;
		uint8_t dat[8];
		uint8_t len;
		PANDA_CAN_PORT bus;
		bool is_receipt;
		bool addr_29b;
	} PANDA_CAN_MSG;

	// This class is exported from the panda.dll
	class PANDA_API Panda {
	public:
		static std::vector<std::string> listAvailablePandas();
		static std::unique_ptr<Panda> openPanda(std::string sn);

		~Panda();

		std::string get_usb_sn();
		bool set_alt_setting(UCHAR alt_setting);
		UCHAR get_current_alt_setting();

		PANDA_HEALTH get_health();
		bool enter_bootloader();
		std::string get_version();
		std::string get_serial();
		std::string get_secret();

		bool set_usb_power(bool on);
		bool set_esp_power(bool on);
		bool esp_reset(uint16_t bootmode);
		bool set_safety_mode(PANDA_SAFETY_MODE mode);
		bool set_can_forwarding(PANDA_CAN_PORT from_bus, PANDA_CAN_PORT to_bus);
		bool set_gmlan(PANDA_GMLAN_HOST_PORT bus);
		bool set_can_loopback(bool enable);
		bool set_can_speed_kbps(PANDA_CAN_PORT bus, uint16_t speed);
		bool set_uart_baud(PANDA_SERIAL_PORT uart, uint32_t rate);
		bool set_uart_parity(PANDA_SERIAL_PORT uart, PANDA_SERIAL_PORT_PARITY parity);

		bool can_send_many(const std::vector<PANDA_CAN_MSG>& can_msgs);
		bool can_send(uint32_t addr, bool addr_29b, const uint8_t *dat, uint8_t len, PANDA_CAN_PORT bus);
		bool can_recv_async(HANDLE kill_event, std::vector<PANDA_CAN_MSG>& msg_buff, DWORD timeoutms = INFINITE);
		std::vector<PANDA_CAN_MSG> can_recv();
		bool can_clear(PANDA_CAN_PORT_CLEAR bus);

		std::string serial_read(PANDA_SERIAL_PORT port_number);
		int serial_write(PANDA_SERIAL_PORT port_number, const void* buff, uint16_t len);
		bool serial_clear(PANDA_SERIAL_PORT port_number);
	private:
		Panda(
			WINUSB_INTERFACE_HANDLE WinusbHandle,
			HANDLE DeviceHandle,
			tstring devPath_,
			std::string sn_
		);

		int control_transfer(
			uint8_t bmRequestType,
			uint8_t bRequest,
			uint16_t wValue,
			uint16_t wIndex,
			void * data,
			uint16_t wLength,
			unsigned int timeout
		);

		int bulk_write(
			UCHAR endpoint,
			const void * buff,
			ULONG length,
			PULONG transferred,
			ULONG timeout
		);

		int Panda::bulk_read(
			UCHAR endpoint,
			void * buff,
			ULONG buff_size,
			PULONG transferred,
			ULONG timeout
		);

		WINUSB_INTERFACE_HANDLE usbh;
		HANDLE devh;
		tstring devPath;
		std::string sn;
		bool loopback;
	};

}
