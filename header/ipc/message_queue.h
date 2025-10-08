#ifndef Q_MSG_H
#define Q_MSG_H

int create_queue(key_t k);
void send_msg(int qid, void msg);
void receive_msg(int qid, void msg, long mtype);
void delete_queue(int qid);

#endif