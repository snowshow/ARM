#ifndef _CAN_H
#define _CAN_H

#include <stdint.h> /* Pour uint8_t */

/**********************************/
/* REPRÉSENTATION DES PACKETS CAN */
/**********************************/

/** Pour homogénéiser la manipulation des packets CAN par les différentes
 * fonctions, une structure commune est définit : la structure can_t (can type).
 * Elle contient l'identifiant du packet sur 11 bits (de 0 à 2047), la
 * longueur du message (de 0 à 8 octets de données) et 8 uint8_t (nombre sur 8
 * bits, équivalent d'un unsigned char) pour chaque éventuel octet de donnée.
 * L'initialisation de cette structure se fait par la fonction CAN_packet.
 * Celle-ci présente l'avantage de vérifier la validité des valeurs passé en
 * argument. Il est dont fortement conseillé de l'utiliser plutôt que de
 * modifier manuellement la structure. En revanche, on pourra utiliser
 * directement la structure à la lecture des valeurs. */

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


/********************************/
/* MANIPULATION DES PACKETS CAN */
/********************************/

/** Initializer la structure can_t à partir des informations passé en argument.
 * @param packet Pointeur sur le packet à initialiser.
 * @param id Identifiant du packet (de 0 à 2047 selon la norme CAN).
 * @param length Nombre d'octet de donnée (de 0 à 8 selon la norme CAN).
 * Paramètre supplémentaire : de 0 à 8 uint8_t, 1 par octet de donnée.
 * @return 0 en cas de succès, -1 en cas d'erreur.
 * Dans ce dernier cas, errno est modifier pour indiquer l'erreur:
 * EINTVAL: Mauvais argument */
int CAN_packet(can_t * packet, int id, int length, ...);

/** Remarques pour les fonctions de manipulation de bits :
 * (CAN_get et CAN_set)
 * Ces fonctions permettent d'accèder à un bit du packet à partir de son index
 * situé dans une variable, par exemple dans une boucle for. Il est recommandé
 * d'accéder aux données directement par la structure lorsque cela est possible.
 * Tolérance des arguments :
 * → L'index doit être dans l'intervalle [0;7]
 * → Si l'index est supérieur ou égale à la taille du packet, la valeur
 * peut-être modifié, et en cas de lecture, la valeur renvoyé sera, sauf cas
 * particulié, non initialisé.
 * →  Si l'index est hors de l'intervalle [0;7], vous vous exposez à recevoivr
 * une erreur de segmentation.
 * → La nouvelle valeur doit être un uint8_t. Si vous passez un int, au mieux le
 * compilateur vous le dira, au pire votre valeur sera tronqué sans que vous
 * en soyez informés. Bref: vérifiez vos valeurs. */

/** Obtenir le b-ième bit du packet dont le pointeur est passé en argument.
 * @param packet Pointeur sur le packet à lire.
 * @param b Index du bit dont la valeur est souhaité, dans l'intervalle [0;7].
 */
uint8_t CAN_get(can_t const * packet, int b);

/** Définir le b-ième bit du packet dont le pointeur est passé en argument.
 * @param packet Pointeur sur le packet à modifier.
 * @param b Index du bit dont la valeur est à modifier.
 * @param c Nouvelle valeur pour le bit à modifier. */
void CAN_set(can_t * packet, int b, uint8_t c);

/** Définition de l'enum can_f (can format) représentant les différents format
 * de données représentant les informations circulant sur le bus CAN */
typedef enum {
	bin, /* Binaire : ce qui circule sur le bus série */
	dec, /* Décimal : ex: 1235	12 23 42 (notez une tabulation puis des espaces) */
	hex, /* Hexadécimal : ex: 1235	0C 17 2A */
	txt /* Texte : les valeurs sont converties en chaînes de textes significatives */
} can_f;


/****************************/
/* RECEPTION DE DONNÉES CAN */
/****************************/

/** La réception des données CAN se fait en deux temps :
 * → L'enregistrement d'un flux à écouter, ainsi que le format des données
 * circulant sur celui-ci, afin de pouvoir reconnaître la forme des packets CAN.
 * → L'enregistrement d'une ou plusieurs fonctions de callback, qui seront
 * appelé à la réception d'un packet, avec celui-ci en argument. Il est possible
 * de définir pour chaque fonction un masque et un filtre sur l'identifiant afin
 * de permettre l'exécution de fonction distinct suivant le type de packet. Les
 * fonctions concerné par un packet sont exécuté les unes après les autres (il
 * pourrat si nécessaire être implémenté une exécution parallalèle de
 * celles-ci). L'ordre d'exécution n'est pas garanti.
 * → Il est également possible de désenregistrer une fonction de callback, grâce
 * à l'identifiant passé en retour de la fonction d'enregistrement.
 * → Il est facultativement possible d'enregistrer une fonction de callback
 * appelé sur problème d'écoute sur le descripteur de fichier. */

/** Placer en écoute un descripteur de fichier.
 * Cette fonction peut-être appelé plusieurs fois. Dans ce cas, le thread
 * d'écoute est tué, les éventuelles fonctions de callback exécutées sont
 * brutalement interrompu, tenez en compte. Celui-ci est alors relancé en écoute
 * sur le nouveau descripteur de fichier. Si le descripteur est négatif, il
 * n'est pas relancé.
 * @param fd File descriptor à écouter.
 * @param format Format des données reçu */
int CAN_listen_on(int fd, can_f format);

/** Enregistrer une fonction de callback.
 * @param mask Masque appliqué à l'identifiant
 * @param filter Filtre auquel est comparé l'identifiant après masquage.
 * @return	-1 en cas d'échec de realloc (errno peut être consulter)
 *		ou	l'indice positif de la fonction de callback (utile au désenregistrement)
 */
int CAN_add_callback(int mask, int filter, void (*event)(can_t));

/** Supprimmer une fonction de callback */
int CAN_rm_callback(int id);

/** Enregistrer une fonction de callback sur erreur. */
int CAN_on_error(void (*fct)(int));

/** Envoyé dans un descripteur un packet au format spécifier. */
int CAN_write(int fd, can_t const * packet, int format);

#endif
