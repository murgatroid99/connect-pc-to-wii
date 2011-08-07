/*
 *  sdp HID keyboard
 *
 *  (c) Collin R. Mulliner <collin@betaversion.net>
 *  http://www.mulliner.org/bluetooth/
 *
 *  License: GPLv2
 *
 *  Parts taken from BlueZ(.org)
 * 
 */

/*
 * I found this source at http://web.archive.org/web/20090418040040/http://www.wiili.org/Wii_bluetooth_specs
 *
 * Edited my Michael Lumish <michael.lumish@gmail.com>
 * 
 * License GPLv3
 */

#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <netinet/in.h>
#include <netdb.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>
#include <openobex/obex.h>

sdp_record_t *sdp_record;
sdp_session_t *sdp_session;

/*
 *  100% taken from bluez-utils (sdptool)
 */
static void add_lang_attr(sdp_record_t *r)
{
	sdp_lang_attr_t base_lang;
	sdp_list_t *langs = 0;

	/* UTF-8 MIBenum (http://www.iana.org/assignments/character-sets) */
	base_lang.code_ISO639 = (0x65 << 8) | 0x6e;
	base_lang.encoding = 106;
	base_lang.base_offset = SDP_PRIMARY_LANG_BASE;
	langs = sdp_list_append(0, &base_lang);
	sdp_set_lang_attr(r, langs);
	sdp_list_free(langs, 0);
}


void sdp_add_hid_attr()
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, hidkb_uuid, l2cap_uuid, hidp_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[3];
	sdp_data_t *channel, *lang_lst, *lang_lst2, *hid_spec_lst, *hid_spec_lst2;
	int i;
	uint8_t dtd = SDP_UINT16;
	uint8_t dtd2 = SDP_UINT8;
	uint8_t dtd_data = SDP_TEXT_STR8;
	sdp_session_t *session;
	void *dtds[2];
	void *values[2];
	void *dtds2[2];
	void *values2[2];
	int leng[2];
	uint8_t hid_spec_type = 0x22;
	uint16_t hid_attr_lang[] = {0x409,0x100};

	static const uint16_t ctrl = 0x11;
	static const uint8_t intr = 0x13;

    // DeviceReleaseNum     0x200
    // ParserVersion        0x201
    // DeviceSubclass       0x202
    // CountryCode          0x203
    // VirtualCable         0x204
    // ReconnectInitiate    0x205
	static const uint16_t hid_attr[] = {0x100,0x111,0x4,0x33,0x0,0x1};

    // SDPDisable           0x208
    // RemoteWakeup         0x20a
    // ProfileVersion       0x20b
    // SupervisionTimeout   0x20c
    // NormallyConnectable  0x20d
    // BootDevice           0x20e
	static const uint16_t hid_attr2[] = {0x0,0x1,0x01,0x100,0xc80,0x00,0x00};
	// taken from: sdptool records --tree xx:xx:xx:xx:xx
	const uint8_t hid_spec[] = { 
        0x05, 
        0x01, 
        0x09, 
        0x05, 
        0xa1, 
        0x01, 
        0x85, 
        0x10, 
        0x15, 
        0x00, 
        0x26, 
        0xff, 
        0x00, 
        0x75, 
        0x08, 
        0x95, 
        0x01, 
        0x06, 
        0x00, 
        0xff, 
        0x09, 
        0x01, 
        0x91, 
        0x00, 
        0x85, 
        0x11, 
        0x95, 
        0x01, 
        0x09, 
        0x01, 
        0x91, 
        0x00, 
        0x85, 
        0x12, 
        0x95, 
        0x02, 
        0x09, 
        0x01, 
        0x91, 
        0x00, 
        0x85, 
        0x13, 
        0x95, 
        0x01, 
        0x09, 
        0x01, 
        0x91, 
        0x00, 
        0x85, 
        0x14, 
        0x95, 
        0x01, 
        0x09, 
        0x01, 
        0x91, 
        0x00, 
        0x85, 
        0x15, 
        0x95, 
        0x01, 
        0x09, 
        0x01, 
        0x91, 
        0x00, 
        0x85, 
        0x16, 
        0x95, 
        0x15, 
        0x09, 
        0x01, 
        0x91, 
        0x00, 
        0x85, 
        0x17, 
        0x95, 
        0x06, 
        0x09, 
        0x01, 
        0x91, 
        0x00, 
        0x85, 
        0x18, 
        0x95, 
        0x15, 
        0x09, 
        0x01, 
        0x91, 
        0x00, 
        0x85, 
        0x19, 
        0x95, 
        0x01, 
        0x09, 
        0x01, 
        0x91, 
        0x00, 
        0x85, 
        0x1a, 
        0x95, 
        0x01, 
        0x09, 
        0x01, 
        0x91, 
        0x00, 
        0x85, 
        0x20, 
        0x95, 
        0x06, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0x85, 
        0x21, 
        0x95, 
        0x15, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0x85, 
        0x22, 
        0x95, 
        0x04, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0x85, 
        0x30, 
        0x95, 
        0x02, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0x85, 
        0x31, 
        0x95, 
        0x05, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0x85, 
        0x32, 
        0x95, 
        0x0a, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0x85, 
        0x33, 
        0x95, 
        0x11, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0x85, 
        0x34, 
        0x95, 
        0x15, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0x85, 
        0x35, 
        0x95, 
        0x15, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0x85, 
        0x36, 
        0x95, 
        0x15, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0x85, 
        0x37, 
        0x95, 
        0x15, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0x85, 
        0x3d, 
        0x95, 
        0x15, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0x85, 
        0x3e, 
        0x95, 
        0x15, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0x85, 
        0x3f, 
        0x95, 
        0x15, 
        0x09, 
        0x01, 
        0x81, 
        0x00, 
        0xc0         // end tag
	};


	if (!sdp_session) {
		printf("%s: sdp_session invalid\n", (char*)__func__);
		exit(-1);
	}
	session = sdp_session;

	sdp_record = sdp_record_alloc();
	if (!sdp_record) {
		perror("add_keyboard sdp_record_alloc: ");
		exit(-1);
	}

	memset((void*)sdp_record, 0, sizeof(sdp_record_t));
	sdp_record->handle = 0xffffffff;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(sdp_record, root);

	add_lang_attr(sdp_record);
	
	sdp_uuid16_create(&hidkb_uuid, HID_SVCLASS_ID);     // Create classID to 0x1124
	svclass_id = sdp_list_append(0, &hidkb_uuid);       // Add to list
	sdp_set_service_classes(sdp_record, svclass_id);    // Set current record to HID

	sdp_uuid16_create(&profile[0].uuid, HID_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, profile);
	sdp_set_profile_descs(sdp_record, pfseq);

	// PROTO
	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[1] = sdp_list_append(0, &l2cap_uuid);
	channel = sdp_data_alloc(SDP_UINT16, &ctrl);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(0, proto[1]);

	sdp_uuid16_create(&hidp_uuid, HIDP_UUID);
	proto[2] = sdp_list_append(0, &hidp_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(sdp_record, aproto);

	// ATTR_ADD_PROTO
	proto[1] = sdp_list_append(0, &l2cap_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &intr);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(0, proto[1]);

	sdp_uuid16_create(&hidp_uuid, HIDP_UUID);
	proto[2] = sdp_list_append(0, &hidp_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_add_access_protos(sdp_record, aproto);
	
	sdp_set_info_attr(sdp_record, "Nintendo RVL-CNT-01", 
		"Nintendo", "Nintendo RVL-CNT-01");

	for (i = 0; i < sizeof(hid_attr)/2; i++) {
		sdp_attr_add_new(sdp_record, SDP_ATTR_HID_DEVICE_RELEASE_NUMBER+i, SDP_UINT16, &hid_attr[i]);
	}

	dtds[0] = &dtd2;
	values[0] = &hid_spec_type;
	dtds[1] = &dtd_data;
	values[1] = (uint8_t*)hid_spec;
	leng[0] = 0;
	leng[1] = sizeof(hid_spec);
	hid_spec_lst = sdp_seq_alloc_with_length(dtds, values, leng, 2);
	hid_spec_lst2 = sdp_data_alloc(SDP_SEQ8, hid_spec_lst);	
	sdp_attr_add(sdp_record, SDP_ATTR_HID_DESCRIPTOR_LIST, hid_spec_lst2);
	
	for (i = 0; i < sizeof(hid_attr_lang)/2; i++) {
		dtds2[i] = &dtd;
		values2[i] = &hid_attr_lang[i];
	}
	lang_lst = sdp_seq_alloc(dtds2, values2, sizeof(hid_attr_lang)/2);
	lang_lst2 = sdp_data_alloc(SDP_SEQ8, lang_lst);	
	sdp_attr_add(sdp_record, SDP_ATTR_HID_LANG_ID_BASE_LIST, lang_lst2);


	
    for (i = 0; i < sizeof(hid_attr2)/2; i++) 
    {
		sdp_attr_add_new(sdp_record, SDP_ATTR_HID_SDP_DISABLE+i, SDP_UINT16, &hid_attr2[i]);
	}
	
	if (sdp_record_register(session, sdp_record, 0) < 0) {
		printf("%s: HID Device (Keyboard) Service Record registration failed\n", (char*)__func__);
		exit(-1);
	}
}


/*
 *  add keyboard descriptor
 */
void sdp_add_device_attr()
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, hidkb_uuid, l2cap_uuid, hidp_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[3];
	sdp_data_t *channel, *lang_lst, *lang_lst2, *hid_spec_lst, *hid_spec_lst2;
	int i;
	uint8_t dtd = SDP_UINT16;
	uint8_t dtd2 = SDP_UINT8;
	uint8_t dtd_data = SDP_TEXT_STR8;
	sdp_session_t *session;
	void *dtds[2];
	void *values[2];
	void *dtds2[2];
	void *values2[2];
	int leng[2];
	uint8_t hid_spec_type = 0x22;
	uint16_t hid_attr_lang[] = {0x409,0x100};

	static const uint8_t ctrl = 0x01;
	static const uint8_t intr = 0x13;

    // DeviceReleaseNum     0x200
    // ParserVersion        0x201
    // DeviceSubclass       0x202
    // CountryCode          0x203
    // VirtualCable         0x204
    // ReconnectInitiate    0x205
	static const uint16_t hid_attr[] = {0x100,0x57e,0x306,0x3a16,0x1,0x2};

    // SDPDisable           0x208
    // RemoteWakeup         0x20a
    // ProfileVersion       0x20b
    // SupervisionTimeout   0x20c
    // NormallyConnectable  0x20d
    // BootDevice           0x20e
	static const uint16_t hid_attr2[] = {0x0,0x01,0x100,0xc80,0x00,0x00};
	// taken from: sdptool records --tree xx:xx:xx:xx:xx
	

	if (!sdp_session) {
		printf("%s: sdp_session invalid\n", (char*)__func__);
		exit(-1);
	}
	session = sdp_session;

	sdp_record = sdp_record_alloc();
	if (!sdp_record) {
		perror("add_keyboard sdp_record_alloc: ");
		exit(-1);
	}

	memset((void*)sdp_record, 0, sizeof(sdp_record_t));
	sdp_record->handle = 0xffffffff;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(sdp_record, root);

	//add_lang_attr(sdp_record);
	
	sdp_uuid16_create(&hidkb_uuid, PNP_INFO_SVCLASS_ID);     // Create classID to 0x1124
	svclass_id = sdp_list_append(0, &hidkb_uuid);       // Add to list
	sdp_set_service_classes(sdp_record, svclass_id);    // Set current record to HID

	sdp_uuid16_create(&profile[0].uuid, PNP_INFO_PROFILE_ID);
	profile[0].version = 0x0100;
	pfseq = sdp_list_append(0, profile);
	sdp_set_profile_descs(sdp_record, pfseq);

	// PROTO
	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[1] = sdp_list_append(0, &l2cap_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &ctrl);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(0, proto[1]);

	sdp_uuid16_create(&hidp_uuid, SDP_UUID);
	proto[2] = sdp_list_append(0, &hidp_uuid);
	apseq = sdp_list_append(apseq, proto[2]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(sdp_record, aproto);

	for (i = 0; i < sizeof(hid_attr)/2; i++) {
		sdp_attr_add_new(sdp_record, SDP_ATTR_HID_DEVICE_RELEASE_NUMBER+i, SDP_UINT16, &hid_attr[i]);
	}

	if (sdp_record_register(session, sdp_record, 0) < 0) {
		printf("%s: HID Device (Keyboard) Service Record registration failed\n", (char*)__func__);
		exit(-1);
	}
}

void sdp_remove()
{
	if (sdp_record && sdp_record_unregister(sdp_session, sdp_record)) {
		printf("%s: HID Device (Keyboard) Service Record unregistration failed\n", (char*)__func__);
	}
	
	sdp_close(sdp_session);
}

int sdp_open()
{
	if (!sdp_session) {
		sdp_session = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, 0);
	}
	if (!sdp_session) {
		printf("%s: sdp_session invalid\n", (char*)__func__);
		//exit(-1);
	}
	else {
		return(1);
	}
	return(0);
}


int main()
{
	sdp_open();
	sdp_add_hid_attr();
	sdp_add_device_attr();
	//sleep(60);	
	//sdp_remove();
	return(1);
}

