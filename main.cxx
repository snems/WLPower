#include <array>
#include <cstdint>
#include <cstdlib>
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <hidapi.h>
#include <stdexcept>

#define MAX_TRY 32

struct WLPower
{
    uint16_t vendorID;
    uint16_t productID;
    int index;
    bool showInfo;
    bool power;
};

class HIDLib 
{
    public:
        HIDLib ()
        {
            if (hid_init())
            {
                throw std::runtime_error("Couldn't initialize libhid");
            }
        }
        ~HIDLib ()
        {
            if (handle)
            {
                hid_close(handle);
            }
		    hid_exit();
        }
    public:
        void open(uint16_t vid, uint16_t pid, int index)
        {
	        struct hid_device_info *devs;
	        const char *path = nullptr;
	        devs = hid_enumerate(0x36A7, 0xA885);
	        for (hid_device_info *cur_dev = devs; cur_dev; cur_dev = cur_dev->next) 
	        {
	        	if (cur_dev->interface_number == index)
	        	{
	        		path = cur_dev->path;
	        	}
	        }

	        if (path)
	        {
	        	handle = hid_open_path(path);
	        }
	        hid_free_enumeration(devs);
	        if (handle == nullptr)
	        {
                throw std::runtime_error("Couldn't open device");
	        }
        }

        void showInfo (void)
        {
	        unsigned char descriptor[HID_API_MAX_REPORT_DESCRIPTOR_SIZE];
	        int res = 0;

	        std::cout << "  Report Descriptor: ";
	        res = hid_get_report_descriptor(handle, descriptor, sizeof(descriptor));
	        if (res < 0) {
                throw std::runtime_error("Couldn't get descriptor report");
	        }
	        else {
                std::cout << "(" << res << "bytes)" << std::endl;
	        }
	        for (int i = 0; i < res; i++) 
	        {
		        if (i % 10 == 0) {
                    std::cout << std::endl;
		        }
                std::cout << "0x" <<  std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(descriptor[i]) << " ";
	        }
            std::cout << std::endl;
        }
        void setReport(std::array<uint8_t, 65> send_to)
        {
	        int rc = hid_send_feature_report(handle, send_to.data(), send_to.size());
	        if (rc != send_to.size())
	        {
                throw std::runtime_error("Couldn't send feature report");
	        }
        }
        void getReport(std::array<uint8_t, 65>& send_to)
        {
	        int rc = hid_get_feature_report(handle, send_to.data(), send_to.size());
	        if (rc != send_to.size())
	        {
                throw std::runtime_error("Couldn't get feature report");
	        }
        }
    private:
	    hid_device *handle{nullptr};
};

int main(int argc, char* argv[])
{
    int c;
    int digit_optind = 0;
    WLPower wp{0x36A7, 0xA885, 2, false, false};

    while (true) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"help", 0, 0, 'h'},
            {"info", 0, 0, 'i'},
            {"power", 0, 0, 'p'},
            {0, 0, 0, 0}
        };

        c = getopt_long (argc, argv, "hi", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'i':
            wp.showInfo = true;
            break;
        case 'p':
            wp.power = true;
            break;

        case '?':
        case 'h':
            std::cout << "Usage:" << std::endl;
            std::cout << "wlpower [--info] [--help] [--power]" << std::endl;
            return EXIT_SUCCESS;

        default:
            return EXIT_FAILURE;
        }
    }

    {
        int count = MAX_TRY;
        HIDLib hidlib;
        bool sleep;
        bool charge;
        int power;
        hidlib.open(wp.vendorID, wp.productID, 2);
        if (wp.showInfo)
        {
            hidlib.showInfo();
        }
        else
        {
            std::array<uint8_t, 65> report;
            while(count)
            {
                hidlib.setReport({ 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x83 });
                report.fill(0);
                hidlib.getReport(report);
                if (report[1] == 0xa1 && report[6] == 0x83)
                {
                    break;
                }
                --count;
            }
            sleep = count == 0;
            if (!sleep)
            {
                charge = report[7];
                power = static_cast<int>(report[8]);
            }
            else
            {
                charge = false;
                power = 0;
            }

            if (wp.power)
            {
                std::cout << power << std::endl;
            }
            else
            {
              std::cout << "Sleep: " << (sleep ? "yes" : "no") << std::endl;
              std::cout << "Charging: " << (charge ? "yes" : "no") << std::endl;
              std::cout << "Power: " << power << "%" << std::endl;
            }
        }
    }

    return EXIT_SUCCESS;
}



