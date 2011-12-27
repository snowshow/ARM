#ifndef _CAN_H
#define _CAN_H

typedef struct {
	int id;
	int length;
	uint8_t* data;
} can_t;


/** Renvoit une structure can_t initialisé à partir des données passées en
 * arguments. Échoue si l'espace mémoire disponible est insuffisant. Renvoit
 * alors NULL et errno contient ENOMEM.
 * @param id Id du message CAN.
 * @param length Nombre d'octets de donnée du packet.
 */
can_t* CAN_packet(int id, int length, ...);

int CAN_send_packet(int fd, can_t const * packet);

#endif
