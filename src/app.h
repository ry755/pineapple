// TODO: clean this up. this is a huge hack to get the old lupos-based code to build
#ifndef _square_app_h
#define _square_app_h

#include <circle/actled.h>
#include <circle/string.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/nulldevice.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/screen.h>
#include <circle/serial.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/usb/usbhcidevice.h>
#include <SDCard/emmc.h>
#include <circle/input/console.h>
#include <circle/sched/scheduler.h>
#include <circle/net/netsubsystem.h>
#include <wlan/bcm4343.h>
#include <wlan/hostap/wpa_supplicant/wpasupplicant.h>

#include <circle_glue.h>
#include <cstring>
#include <stdexcept>

/**
 * Basic Circle Stdlib application that supports GPIO access.
 */
class CStdlibApp
{
public:
        enum TShutdownMode
        {
                ShutdownNone,
                ShutdownHalt,
                ShutdownReboot
        };

        CStdlibApp (const char *kernel) :
                FromKernel (kernel)
        {
                if (FromKernel == nullptr)
                {
                        throw std::invalid_argument ("CStdlibApp: kernel name must not be nullptr");
                }
        }

        virtual ~CStdlibApp (void)
        {
        }

        virtual bool Initialize (void)
        {
                return mInterrupt.Initialize ();
        }

        virtual void Cleanup (void)
        {
        }

        virtual TShutdownMode Run (void) = 0;

        const char *GetKernelName(void) const
        {
                return FromKernel;
        }

protected:
        CActLED            mActLED;
        CKernelOptions     mOptions;
        CDeviceNameService mDeviceNameService;
        CNullDevice        mNullDevice;
        CExceptionHandler  mExceptionHandler;
        CInterruptSystem   mInterrupt;

private:
        char const *FromKernel;
};

/**
 * Stdlib application that adds screen support
 * to the basic CStdlibApp features.
 */
class CStdlibAppScreen : public CStdlibApp
{
public:
        CStdlibAppScreen(const char *kernel)
                : CStdlibApp (kernel),
                  mScreen (mOptions.GetWidth (), mOptions.GetHeight ()),
                  mTimer (&mInterrupt),
                  mLogger (mOptions.GetLogLevel (), &mTimer)
        {
        }

        virtual bool Initialize (void)
        {
                if (!CStdlibApp::Initialize ())
                {
                        return false;
                }

                if (!mScreen.Initialize ())
                {
                        return false;
                }

                if (!mSerial.Initialize (115200))
                {
                        return false;
                }

                CDevice *pTarget =
                        mDeviceNameService.GetDevice (mOptions.GetLogDevice (), false);
                if (pTarget == 0)
                {
                        pTarget = &mScreen;
                }

                if (!mLogger.Initialize (pTarget))
                {
                        return false;
                }

                return mTimer.Initialize ();
        }

        CScreenDevice   mScreen;
        CSerialDevice   mSerial;
        CTimer          mTimer;
        CLogger         mLogger;
};

/**
 * Stdlib application that adds stdio support
 * to the CStdlibAppScreen functionality.
 */
class CSquareApp: public CStdlibAppScreen
{
private:
        char const *mpPartitionName;

public:
        // TODO transform to constexpr
        // constexpr char static DefaultPartition[] = "emmc1-1";
#define CSTDLIBAPP_LEGACY_DEFAULT_PARTITION "emmc1-1"
#define CSTDLIBAPP_DEFAULT_PARTITION "SD:"

        CSquareApp (const char *kernel,
                         const char *pPartitionName = CSTDLIBAPP_DEFAULT_PARTITION)
                : CStdlibAppScreen (kernel),
                  mpPartitionName (pPartitionName),
                  mUSBHCI (&mInterrupt, &mTimer, TRUE),
                  mEMMC (&mInterrupt, &mTimer, &mActLED),
                  mConsole (0, TRUE)
        {
                if (mpPartitionName == nullptr)
                {
                        throw std::invalid_argument ("CStdlibAppStdio: pPartitionName must not be nullptr");
                }
        }

        virtual bool Initialize (void)
        {
                if (!CStdlibAppScreen::Initialize ())
                {
                        return false;
                }

                if (!mEMMC.Initialize ())
                {
                        return false;
                }

                char const *partitionName = mpPartitionName;

                // Recognize the old default partition name
                if (strcmp(partitionName, CSTDLIBAPP_LEGACY_DEFAULT_PARTITION) == 0)
                {
                        partitionName = CSTDLIBAPP_DEFAULT_PARTITION;
                }

                if (f_mount (&mFileSystem, partitionName, 1) != FR_OK)
                {
                        mLogger.Write (GetKernelName (), LogError,
                                         "Cannot mount partition: %s", partitionName);

                        return false;
                }

                if (!mUSBHCI.Initialize ())
                {
                        return false;
                }

                if (!mConsole.Initialize ())
                {
                        return false;
                }

                // Initialize newlib stdio with a reference to Circle's console
                CGlueStdioInit (mConsole);

                mLogger.Write (GetKernelName (), LogNotice, "Compile time: " __DATE__ " " __TIME__);

                return true;
        }

        virtual void Cleanup (void)
        {
                f_mount(0, "", 0);

                CStdlibAppScreen::Cleanup ();
        }

        const char *GetPartitionName() const
        {
                return mpPartitionName;
        }

        CUSBHCIDevice   mUSBHCI;
        CEMMCDevice     mEMMC;
        FATFS           mFileSystem;
        CConsole        mConsole;
};

/**
 * Stdlib application that adds network functionality
 * to the CStdlibAppStdio features.
 */
class CStdlibAppNetwork: public CSquareApp
{
public:
        #define CSTDLIBAPP_WLAN_FIRMWARE_PATH   CSTDLIBAPP_DEFAULT_PARTITION "/firmware/"
        #define CSTDLIBAPP_WLAN_CONFIG_FILE     CSTDLIBAPP_DEFAULT_PARTITION "/wpa_supplicant.conf"

        CStdlibAppNetwork (const char *kernel,
                   const char *pPartitionName = CSTDLIBAPP_DEFAULT_PARTITION,
                   const u8 *pIPAddress      = 0,       // use DHCP if pIPAddress == 0
                   const u8 *pNetMask        = 0,
                   const u8 *pDefaultGateway = 0,
                   const u8 *pDNSServer      = 0,
                   TNetDeviceType DeviceType = NetDeviceTypeEthernet)
          : CSquareApp(kernel, pPartitionName),
            mDeviceType (DeviceType),
            mWLAN (CSTDLIBAPP_WLAN_FIRMWARE_PATH),
            mNet(pIPAddress, pNetMask, pDefaultGateway, pDNSServer, DEFAULT_HOSTNAME, DeviceType),
            mWPASupplicant (CSTDLIBAPP_WLAN_CONFIG_FILE)
        {
        }

        virtual bool Initialize (bool const bWaitForActivate = true)
        {
                if (!CSquareApp::Initialize ())
                {
                        return false;
                }

                if (mDeviceType == NetDeviceTypeWLAN)
                {
                        if (!mWLAN.Initialize ())
                        {
                                return false;
                        }
                }

                if (!mNet.Initialize (false))
                {
                        return false;
                }

                if (mDeviceType == NetDeviceTypeWLAN)
                {
                        if (!mWPASupplicant.Initialize ())
                        {
                                return false;
                        }
                }

                while (bWaitForActivate && !mNet.IsRunning ())
                {
                        mScheduler.Yield ();
                }

                return true;
        }

protected:
        CScheduler      mScheduler;
        TNetDeviceType  mDeviceType;
        CBcm4343Device  mWLAN;
        CNetSubSystem   mNet;
        CWPASupplicant  mWPASupplicant;
};
#endif
