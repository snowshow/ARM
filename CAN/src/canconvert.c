#define LONG_OPTIONS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef LONG_OPTIONS
#include <getopt.h>
#endif
#include <string.h>

#include "libcan.h"

/* Env variable name */
#define ENV_INPUT "CONV_INPUT"
#define ENV_OUTPUT "CONV_OUTPUT"

/* Default value */
#define DEFAULT_INPUT_FORMAT	bin
#define DEFAULT_OUTPUT_FORMAT	dec

typedef struct can_t can_t;
typedef enum can_f can_f;

void listener(can_t packet);

can_f output_format;

int parse_format(can_f * format, char * str)
{
	if (strcmp(str, "bin") == 0) {
		*format = bin;
		return 0;
	}
	if (strcmp(str, "dec") == 0) {
		*format = dec;
		return 0;
	}
	if (strcmp(str, "hex") == 0) {
		*format = hex;
		return 0;
	}

	return -1;
}

void help(char * cmd)
{
#ifdef LONG_OPTIONS
	printf("Usage: %s [-i|--input=<bin|dec|hex>] [-o|--option=<bin|dec|hex] [-h|--help]\n", cmd);
#else
	printf("Usage: %s [-i <bin|dec|hex>] [-o <bin|dec|hex>] [-h]\n", cmd);
#endif
}

int main(int argc, char * argv[])
{
	can_f input_format = DEFAULT_INPUT_FORMAT;
	output_format = DEFAULT_OUTPUT_FORMAT;

	int option;

	char * env;

	env = getenv(ENV_INPUT);
	if ((env != NULL) && (strlen(env) != 0)) {
		parse_format(&input_format, env);
	}

	env = getenv(ENV_OUTPUT);
	if ((env != NULL) && (strlen(env) != 0)) {
		parse_format(&output_format, env);
	}

	opterr = 1;
	while (1) {
#ifdef LONG_OPTIONS
		int index = 0;
		static struct option longopts[] = {
			{ "input",	1,	NULL,	'i' },
			{ "output",	1,	NULL,	'o' },
			{ "help", 0, NULL, 'h' }
		};
		option = getopt_long(argc, argv, "i:o:h", longopts, &index);
#else
		option = getopt(argc, argv, "i:o:h");
#endif
		if (option == -1)
			break;

		switch (option) {
			case 'i':
				parse_format(&input_format, optarg);
				break;
			case 'o':
				parse_format(&output_format, optarg);
				break;
			case 'h':
			case '?':
				help(argv[0]);
				exit(0);
		}
	}

	/*if (CAN_on_exit(exit) < 0){
		perror("CAN_on_exit");
		exit(1);
	}*/

	can_ctx * ctx;
	if (can_init(&ctx) < 0) {
		perror("can_init");
		exit(1);
	}

	if (can_register_callback(ctx, 0, 0, listener) < 0) {
		perror("can_register_callback");
		exit(1);
	}
	if (can_listen_on(ctx, STDIN_FILENO, input_format) < 0) {
		perror("can_listen_on");
		exit(1);
	}

	pause();

	return 0;
}

void listener(can_t packet)
{
	if (can_packet_write(1, output_format, &packet) < 0) {
		perror("CAN_write");
		exit(1);
	}
}
