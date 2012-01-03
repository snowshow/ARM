#ifndef _CAN_H
#define _CAN_H

#include <stdint.h> /* Pour uint8_t */

/** Définition de la structure can_t représentant les packets CAN */
typedef struct {
	int id;
	int length;
	uint8_t b1;
	uint8_t b2;
	uint8_t b3;
	uint8_t b4;
	uint8_t b5;
	uint8_t b6;
	uint8_t b7;
	uint8_t b8;
} can_t;

/** Initialize la structure can_t à partir des informations passé en argument.
 * @param packet Pointeur sur le packet à initialiser.
 * @param id Identifiant du packet (de 0 à 2047 selon la norme CAN).
 * @param length Nombre d'octet de donnée (de 0 à 8 selon la norme CAN).
 * Paramètre supplémentaire : de 0 à 8 uint8_t, 1 par octet de donnée. */
int CAN_packet(can_t * packet, int id, int length, ...);

/** Définition de l'enum can_f (can format) représentant les différents format
 * de données représentant les informations circulant sur le bus CAN */
typedef enum {
	bin,
	dec,
	hex,
	txt
} can_f;

void CAN_set(can_t * packet, int b, uint8_t c);
int CAN_on_event(int mask, int filter, void (*event)(can_t));
int CAN_listen_on(int fd, can_f);
uint8_t CAN_get(can_t const * packet, int b);


int CAN_write(int fd, can_t const * packet, int format);

#endif
