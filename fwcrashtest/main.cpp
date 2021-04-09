//
//  main.cpp
//  fwcrashtest
//
//  Created by Paul Neyrinck on 4/9/21.
//  This demonstrates a kernel panic bug in Mac OS on M1-baesd Mac with Big Sur 11.1.
//  The crash does not happen on Intel based Macs.
//  This example uses the local firewire node so no firewire device needs to be attached to the local port.
//  The panic will also happen if an actual attached device is used.
//  The code here simply creates a pseudo address space and then releases it.
//  The kernel panic happens when the address space is released.

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <unistd.h>

// user client includes:
#include <IOKit/firewire/IOFireWireLib.h>
#include <IOKit/firewire/IOFireWireFamilyCommon.h>

Ptr     gBuf = 0 ;

int
main(int, char**)
{
    IOReturn                error    =    kIOReturnSuccess ;

    mach_port_t                masterPort ;
    error = ::IOMasterPort( MACH_PORT_NULL, & masterPort ) ;

    //
    // get a user client
    //
    
    IOFireWireLibDeviceRef    interface = 0 ;
    IOCFPlugInInterface**    cfPlugInInterface = 0 ;
    
    if ( ! error )
    {
        SInt32         theScore ;
        
        error = ::IOCreatePlugInInterfaceForService(
                        IOServiceGetMatchingService( masterPort, IOServiceMatching( "IOFireWireLocalNode" ) ),
                        kIOFireWireLibTypeID, kIOCFPlugInInterfaceID,
                        & cfPlugInInterface, & theScore) ;
    }
    
    if ( ! error )
    {
        (*cfPlugInInterface)->QueryInterface(
                                    cfPlugInInterface,
                                    CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID),
                                    (void**) & interface) ;
    }

    if ( ! error )
        error = (*interface)->Open(interface) ;
    
    // ask user client to create a pseudo address space for receiving packets
    IOFireWireLibPseudoAddressSpaceRef addressSpace = 0 ;
    
    if ( !error )
    {
        gBuf = (Ptr) new char[40960] ;                        // allocate the backing store
       
        // ask our interface to create a pseudo address space for us
        addressSpace = (*interface)->CreatePseudoAddressSpace(interface, 40960,
                                            (void*) addressSpace, 8192, gBuf,
                                            0,
                                            CFUUIDGetUUIDBytes(kIOFireWirePseudoAddressSpaceInterfaceID)) ;
        
        if (addressSpace)
        {
            FWAddress    address ;
            (*addressSpace)->GetFWAddress(addressSpace, & address) ;
            
            printf("allocated address space at %04x:%08x\n", address.addressHi, (unsigned int)address.addressLo) ;
        }
        else
            error = kIOReturnNoResources ;


    }

    // this Release causes a kernel panic on M1-based Mac
    if (addressSpace)
        ( *addressSpace )->Release( addressSpace ) ;
        
    if ( interface )
    {
        (*interface)->Close(interface) ;
        (*interface)->Release(interface) ;
    }
    
    if (cfPlugInInterface)
        IODestroyPlugInInterface(cfPlugInInterface) ;
    
    return ( !error ) ? EXIT_SUCCESS : EXIT_FAILURE ;
}
