#include <stdio.h>
#include <string>
#include <sys/process.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cell/sysmodule.h>
#include <arpa/inet.h>
#include <netex/net.h>
#include <netex/errno.h>
#include <netdb.h>

void check_value(int32_t ret, int32_t expected, std::string message)
{
	if (ret != expected)
	{
		printf("%s\n", message.c_str());
		sys_process_exit(1);
	}
}

void check_value(bool condition, std::string message)
{
	if (!condition)
	{
		printf("%s\n", message.c_str());
		sys_process_exit(1);
	}
}

int32_t main()
{
	// Disable printf buffering
	setbuf(stdout, NULL);

	printf("TEST00009 by tambre.\n");
	printf("Checking the behaviour of errno and its accuracy.\n\n");

	printf("Loading CELL_SYSMODULE_NET...\n");
	int32_t ret = cellSysmoduleLoadModule(CELL_SYSMODULE_NET);

	check_value(ret, CELL_OK, "cellSysmoduleLoadModule(CELL_SYSMODULE_NET) failed.");
	check_value(sys_net_errno, SYS_NET_EBUSY, "sys_net_errno should be SYS_NET_EBUSY before initialization.");
	check_value(sys_net_h_errno, 0, "sys_net_errno_h should be 0 before initialization.");

	ret = sys_net_initialize_network();
	check_value(ret, CELL_OK, "Failed to initialize networking.");
	check_value(sys_net_errno, 0, "errno after initialization should be 0.");

	// Try to create a socket using bogus type
	int32_t sock = socket(AF_INET, 11, 0);
	printf("socket error: 0x%x", sock);
	check_value(sock < 0, "Creation of socket with invalid type should be unsuccessful.");
	check_value(sys_net_errno, SYS_NET_EPROTONOSUPPORT, "Creation of socket with invalid type should result in sys_net_errno being set to SYS_NET_EPROTONOSUPPORT.");

	// Try to create a proper socket
	sock = socket(AF_INET, SOCK_STREAM_P2P, 0);
	check_value(sock > 0, "Creation of socket with valid type should be successful.");
	check_value(sys_net_errno, SYS_NET_EPROTONOSUPPORT, "Previous error value should persist on successful function call.");

	ret = sys_net_finalize_network();
	check_value(ret, CELL_OK, "Failed to finalize networking.");
	check_value(sys_net_errno, SYS_NET_EBUSY, "sys_net_errno_h should be 16 after finalization.");

	//char* test = inet_ntoa(0x7F000001);
	printf("errno: %d\n", sys_net_errno);

	printf("errno behaviour seems to be correct!\n");

	return 0;
}