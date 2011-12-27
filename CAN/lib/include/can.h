#ifndef _CAN_H
#define _CAN_H

#include <stdint.h>

typedef struct {
	int id;
	int length;
	uint8_t b[8];
} can_t;


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
int CAN_recv(int fd, void (*event)(can_t*));

#endif
