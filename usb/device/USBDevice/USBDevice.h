/* mbed Microcontroller Library
 * Copyright (c) 2018-2018 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef USBDEVICE_H
#define USBDEVICE_H

#include "mbed.h"
#include "USBDevice_Types.h"
#include "USBPhy.h"
#include "mbed_critical.h"

class USBDevice;


class USBDevice: public  USBPhyEvents {
public:
    typedef void (USBDevice::*ep_cb_t)(usb_ep_t endpoint);

    enum RequestResult {
        Receive = 0,
        Send = 1,
        Success = 2,
        Failure = 3,
        PassThrough = 4,
    };

    enum DeviceState {
        Attached,
        Powered,
        Default,
        Address,
        Configured
    };

    struct setup_packet_t {
        struct {
            uint8_t dataTransferDirection;
            uint8_t Type;
            uint8_t Recipient;
        } bmRequestType;
        uint8_t  bRequest;
        uint16_t wValue;
        uint16_t wIndex;
        uint16_t wLength;
    };

    USBDevice(uint16_t vendor_id, uint16_t product_id, uint16_t product_release);
    USBDevice(USBPhy *phy, uint16_t vendor_id, uint16_t product_id, uint16_t product_release);

    /**
     * Initialize this instance
     *
     * This function must be called before calling
     * any other functions of this class, unless specifically
     */
    void init();

    /**
     * Power down this instance
     *
     * Disable interrupts and stop sending events.
     */
    void deinit();

    /**
    * Check if the device is configured
    *
    * @returns true if configured, false otherwise
    */
    bool configured();

    /**
    * Connect a device
    *
    * @param blocking: block if not configured
    */
    void connect(bool blocking = true);

    /**
    * Disconnect a device
    */
    void disconnect();

    /**
     * Enable the start of frame interrupt
     *
     * Call USBDevice::callback_sof on every frame.
     */
    void sof_enable();

    /**
     * Disable the start of frame interrupt
     *
     * Stop calling USBDevice::callback_sof.
     */
    void sof_disable();

    /**
    * Add an endpoint
    *
    * @param endpoint Endpoint to enable
    * @param max_packet Maximum size of a packet which can be sent or received on this endpoint
    * @param type Endpoint type - USB_EP_TYPE_BULK, USB_EP_TYPE_INT or USB_EP_TYPE_ISO
    * @param callback Method pointer to be called when a packet is transferred
    * @returns true if successful, false otherwise
    */
    bool endpoint_add(usb_ep_t endpoint, uint32_t max_packet, usb_ep_type_t type, ep_cb_t callback = NULL);

    /**
    * Add an endpoint
    *
    * @param endpoint Endpoint to enable
    * @param max_packet Maximum size of a packet which can be sent or received on this endpoint
    * @param type Endpoint type - USB_EP_TYPE_BULK, USB_EP_TYPE_INT or USB_EP_TYPE_ISO
    * @param callback Method pointer to be called when a packet is transferred
    * @returns true if successful, false otherwise
    */
    template<typename T>
    bool endpoint_add(usb_ep_t endpoint, uint32_t max_packet, usb_ep_type_t type, void (T::*callback)(usb_ep_t endpoint))
    {
        return endpoint_add(endpoint, max_packet, type, static_cast<ep_cb_t>(callback));
    }

    /**
    * Remove an endpoint
    *
    * @param endpoint Endpoint to disable
    * @note This endpoint must already have been setup with endpoint_add
    */
    void endpoint_remove(usb_ep_t endpoint);

    /**
    * Stall an endpoint
    *
    * @param endpoint Endpoint to stall
    * @note You cannot stall endpoint 0 with this function
    * @note This endpoint must already have been setup with endpoint_add
    */
    void endpoint_stall(usb_ep_t endpoint);

    /**
    * Unstall an endpoint
    *
    * @param endpoint Endpoint to unstall
    * @note This endpoint must already have been setup with endpoint_add
    */
    void endpoint_unstall(usb_ep_t endpoint);

    /**
     * Get the current maximum size for this endpoint
     *
     * Return the currently configured maximum packet size, wMaxPacketSize,
     * for this endpoint.
     * @note This endpoint must already have been setup with endpoint_add
     */
    uint32_t endpoint_max_packet_size(usb_ep_t endpoint);

    /** Start a read on the given endpoint
     *
     * After the read is finished call read_start to get the result.
     *
     * @param endpoint endpoint to perform the read on
     * @return true if the read was started, false if no more reads can be started
     * @note This endpoint must already have been setup with endpoint_add
     */
    bool read_start(usb_ep_t endpoint);

    /**
     * Finish a read on the given endpoint
     *
     * Get the contents of a read started with read_start. To ensure all
     * the data from this endpoint is read make sure the buffer and size
     * passed is at least as big as the maximum packet for this endpoint.
     *
     * @param endpoint endpoint to read data from
     * @param buffer buffer to fill with read data
     * @param max_size the total size of the data buffer. This must be at least
     * the max packet size of this endpoint
     * @param size The size of data that was read
     * @return true if the read was completed, otherwise false
     * @note This endpoint must already have been setup with endpoint_add
     */
    bool read_finish(usb_ep_t endpoint, uint8_t *buffer, uint32_t max_size, uint32_t *size);

    /**
     * Write a data to the given endpoint
     *
     * Write data to an endpoint.
     *
     * @param endpoint endpoint to write data to
     * @param buffer data to write
     * @param size the size of data to send. This must be less than or equal to the
     * max packet size of this endpoint
     * @note This endpoint must already have been setup with endpoint_add
     */
    bool write(usb_ep_t endpoint, uint8_t *buffer, uint32_t size);

    /*
    * Get device descriptor.
    *
    * @returns pointer to the device descriptor
    */
    virtual const uint8_t *device_desc();

    /*
    * Get configuration descriptor
    *
    * @returns pointer to the configuration descriptor
    */
    virtual const uint8_t *configuration_desc()
    {
        return NULL;
    };

    /*
    * Get string lang id descriptor
    *
    * @return pointer to the string lang id descriptor
    */
    virtual const uint8_t *string_langid_desc();

    /*
    * Get string manufacturer descriptor
    *
    * @returns pointer to the string manufacturer descriptor
    */
    virtual const uint8_t *string_imanufacturer_desc();

    /*
    * Get string product descriptor
    *
    * @returns pointer to the string product descriptor
    */
    virtual const uint8_t *string_iproduct_desc();

    /*
    * Get string serial descriptor
    *
    * @returns pointer to the string serial descriptor
    */
    virtual const uint8_t *string_iserial_desc();

    /*
    * Get string configuration descriptor
    *
    * @returns pointer to the string configuration descriptor
    */
    virtual const uint8_t *string_iconfiguration_desc();

    /*
    * Get string interface descriptor
    *
    * @returns pointer to the string interface descriptor
    */
    virtual const uint8_t *string_iinterface_desc();

    /*
    * Get the length of the report descriptor
    *
    * @returns length of the report descriptor
    */
    virtual uint16_t report_desc_dength()
    {
        return 0;
    };

protected:

    /**
    * Called by USBDevice layer on power state change.
    *
    * @param powered true if device is powered, false otherwise
    *
    * Warning: Called in ISR context
    */
    virtual void callback_power(bool powered)
    {

    }

    /**
    * Called by USBDevice layer on each new USB frame.
    *
    * Callbacks are enabled and disabled by calling sof_enable
    * and sof_disable.
    *
    * @param frame_number The current frame number
    *
    * Warning: Called in ISR context
    */
    virtual void callback_sof(int frame_number)
    {

    }

    /**
    * Called by USBDevice layer on bus reset.
    *
    * complete_reset must be called after
    * the device is fully reset.
    *
    * Warning: Called in ISR context
    */
    virtual void callback_reset()
    {

    }

    /**
    * Called when USB changes state
    *
    * @param new_state The new state of the USBDevice
    *
    * Warning: Called in ISR context
    */
    virtual void callback_state_change(DeviceState new_state) = 0;

    /**
    * Called by USBDevice on Endpoint0 request.
    *
    * This is used to handle extensions to standard requests
    * and class specific requests. The function complete_request
    * must be always be called in response to this callback.
    *
    * Warning: Called in ISR context
    */
    virtual void callback_request(const setup_packet_t *setup) = 0;
    void complete_request(RequestResult direction, uint8_t *data=NULL, uint32_t size=0);

    /**
    * Called by USBDevice on data stage completion
    *
    * The function complete_request_xfer_done must be always be called
    * in response to this callback.
    *
    * Warning: Called in ISR context
    */
    virtual void callback_request_xfer_done(const setup_packet_t *setup) = 0;
    void complete_request_xfer_done(bool success);

    /*
    * Called by USBDevice layer in response to set_configuration.
    *
    * Upon reception of this command endpoints of the previous configuration
    * if any must be removed with endpoint_remove and new endpoint added with
    * endpoint_add.
    *
    * @param configuration Number of the configuration
    *
    * Warning: Called in ISR context
    */
    virtual void callback_set_configuration(uint8_t configuration) = 0;
    void complete_set_configuration(bool success);

    /*
    * Called by USBDevice layer in response to set_interface.
    *
    * Upon reception of this command endpoints of any previous interface
    * if any must be removed with endpoint_remove and new endpoint added with
    * endpoint_add.
    *
    * @param configuration Number of the configuration
    *
    * Warning: Called in ISR context
    */
    virtual void callback_set_interface(uint16_t interface, uint8_t alternate) = 0;
    void complete_set_interface(bool success);

    uint8_t *find_descriptor(uint8_t descriptorType);

    const usb_ep_table_t *endpoint_table();

    /**
     * Callback called to indicate the USB processing needs to be done
     */
    virtual void start_process();

    /**
     * Acquire exclusive access to this instance USBDevice
     */
    virtual void lock();

    /**
     * Release exclusive access to this instance USBDevice
     */
    virtual void unlock();

    /**
     * Assert that the current thread of execution holds the lock
     *
     */
    virtual void assert_locked();

    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t product_release;
    uint8_t device_descriptor[18];

private:
    // USBPhyEvents
    virtual void power(bool powered);
    virtual void suspend(bool suspended);
    virtual void sof(int frame_number);
    virtual void reset();
    virtual void ep0_setup();
    virtual void ep0_out();
    virtual void ep0_in();
    virtual void out(usb_ep_t endpoint);
    virtual void in(usb_ep_t endpoint);

    bool request_get_descriptor();
    bool control_out();
    bool control_in();
    bool request_set_address();
    bool request_set_configuration();
    bool request_set_feature();
    bool request_clear_feature();
    bool request_get_status();
    bool request_setup();
    void control_setup();
    void control_abort();
    void control_abort_start();
    void control_setup_continue();
    void decode_setup_packet(uint8_t *data, setup_packet_t *packet);
    bool request_get_configuration();
    bool request_get_interface();
    bool request_set_interface();
    void change_state(DeviceState state);

    struct endpoint_info_t {
        void (USBDevice::*callback)(usb_ep_t endpoint);
        uint16_t max_packet_size;
        uint8_t flags;
        uint8_t pending;
    };

    struct usb_device_t {
        volatile DeviceState state;
        uint8_t configuration;
        bool suspended;
    };

    enum ControlState {
        Setup,
        DataOut,
        DataIn,
        Status
    };

    struct control_transfer_t {
        setup_packet_t setup;
        uint8_t *ptr;
        uint32_t remaining;
        uint8_t direction;
        bool zlp;
        bool notify;
        ControlState stage;
        bool user_callback;
    };

    endpoint_info_t endpoint_info[32 - 2];

    USBPhy *phy;
    bool initialized;
    control_transfer_t transfer;
    usb_device_t device;
    uint32_t max_packet_size_ep0;

    bool setup_ready;
    bool abort_control;

    uint16_t current_interface;
    uint8_t current_alternate;
    uint32_t locked;
};


#endif
