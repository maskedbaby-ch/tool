/* main.c */

/*
    This program demonstrates a simple application of JSON_checker. It reads
    a JSON text from STDIN, producing an error message if the text is rejected.

        % JSON_checker <test/pass1.json
*/

#include <stdlib.h>
#include <stdio.h>
#include "JSON_checker.h"

char *json = "{\"wanconnectionType\":\"dynamic\",\"PPPoE\":{\"enabled\":true,\"username\":\"plume\",\"password\":\"Plume1234!\"},\"DataService\":{\"enabled\":true,\"VLAN\":0,\"QoS\":0},\"staticIPv4\":{\"enabled\":true,\"ip\":\"35.222.58.226\",\"subnet\":\"255.255.255.0\",\"gateway\":\"35.222.58.1\",\"primaryDns\":\"1.1.1.1\",\"secondaryDns\":\"1.0.0.1\"}}";


int main(int argc, char* argv[]) {
/*
    Read STDIN. Exit with a message if the input is not well-formed JSON text.

    jc will contain a JSON_checker with a maximum depth of 20.
*/
	if(json_checker(json) == 0)
	{
		printf("valid json\n");
	}
	else
	{
		printf("invalid json\n");
	}
}
