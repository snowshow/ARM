#ifndef _CAN_H
#define _CAN_H

#include <stdint.h>

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

void CAN_set(can_t * packet, int b, uint8_t c);
int CAN_on_event(int mask, int filter, void (*event)(can_t));
int CAN_listen_on(int fd);
uint8_t CAN_get(can_t const * packet, int b);

/** Renvoit une structure can_t initialisé à partir des données passées en
 * arguments. Échoue si l'espace mémoire disponible est insuffisant. Renvoit
 * alors NULL et errno contient ENOMEM.
 * @param id Id du message CAN.
 * @param length Nombre d'octets de donnée du packet.
 */
int CAN_packet(can_t * packet, int id, int length, ...);

/** Écrit le message can packet sur le file descriptor fd
 * @param fd File descriptor sur lequel sera écrit le packet
 * @param packet Structure contenant les données à transmettre.
 * @return 0 en cas de succès, -1 en cas d'échec (consulter errno)
 */
int CAN_write(int fd, can_t const * packet);

/** Enregistre une fonction qui sera appelé en cas de données can arrivant sur
 * le file descriptor passé en argument.
 * @param fd File descriptor en écoute
 * @param event Pointeur sur une fonction du type void (*event)(can_t*) qui sera
 * appelé en cas de réception de données CAN.
 */
//int CAN_recv(int fd, int mask, void (*event)(can_t));

#endif
