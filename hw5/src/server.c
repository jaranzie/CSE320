//
// Created by jaranzie on 12/5/22.
//
#include <stdlib.h>
#include "server.h"
#include <pthread.h>
#include "protocol.h"
#include "trader.h"
#include <string.h>

void send_nack(int fd) {
    struct brs_packet_header header;
    header.type = BRS_NACK_PKT;
    header.size = 0;
//    header.timestamp_sec =
    proto_send_packet(fd,&header,NULL);
}

void send_ack(int fd) {
    struct brs_packet_header header;
    header.type = BRS_ACK_PKT;
    header.size = 0;
//    header.timestamp_sec =
    proto_send_packet(fd,&header,NULL);
}

void *brs_client_service(void *arg) {
    int fd = *(int*)arg;
    free(arg);
    pthread_detach(pthread_self());
    creg_register(client_registry, fd);
    TRADER* current_trader = NULL;
    struct brs_packet_header header;
    void* data = NULL;
    ACCOUNT *current_account;

    while(1) {
        if(current_trader != NULL) {
            current_account = trader_get_account(current_trader);
        }
        proto_recv_packet(fd, &header, &data);
        uint8_t type = header.type;
        if(current_trader == NULL && type != BRS_LOGIN_PKT) {
            send_nack(fd);
            continue;
        } else {
            struct brs_packet_header response_header;
            BRS_STATUS_INFO current_status;
            quantity_t amount;
            //void* response_payload;
            switch (type) {
                case BRS_STATUS_PKT: {
                    response_header.type = BRS_ACK_PKT;
                    response_header.size = htons((uint16_t) sizeof(BRS_STATUS_INFO));
                    current_account = trader_get_account(current_trader);

                    exchange_get_status(exchange, current_account, &current_status);
                    current_status.balance = htonl(current_status.balance);
                    current_status.inventory = htonl(current_status.inventory);
                    current_status.bid = htonl(current_status.bid);
                    current_status.ask = htonl(current_status.ask);
                    current_status.last = htonl(current_status.last);
                    current_status.orderid = htonl(current_status.orderid);
                    current_status.quantity = htonl(current_status.quantity);

                    if (data != NULL) {
                        free(data);
                    }
                    proto_send_packet(fd, &response_header, &current_status);
                }
                    break;
                case BRS_DEPOSIT_PKT: {
                    if (data == NULL) {
                        send_nack(fd);
                        break;
                    }
                    response_header.type = BRS_ACK_PKT;
                    response_header.size = htons((uint16_t) sizeof(BRS_STATUS_INFO));
                    amount = *(quantity_t *) data;
                    account_increase_balance(trader_get_account(current_trader), amount);

                    exchange_get_status(exchange, current_account, &current_status);
                    current_status.balance = htonl(current_status.balance);
                    current_status.inventory = htonl(current_status.inventory);
                    current_status.bid = htonl(current_status.bid);
                    current_status.ask = htonl(current_status.ask);
                    current_status.last = htonl(current_status.last);
                    current_status.orderid = htonl(current_status.orderid);
                    current_status.quantity = htonl(current_status.quantity);


                    if (data != NULL) {
                        free(data);
                    }
                    proto_send_packet(fd, &response_header, &current_status);
                }
                    break;
                case BRS_WITHDRAW_PKT: {
                    if (data == NULL) {
                        send_nack(fd);
                        break;
                    }
                    amount = *(quantity_t *) data;
                    if (data != NULL) {
                        free(data);
                    }
                    int fail = account_decrease_balance(trader_get_account(current_trader), amount);


                    exchange_get_status(exchange, current_account, &current_status);
                    current_status.balance = htonl(current_status.balance);
                    current_status.inventory = htonl(current_status.inventory);
                    current_status.bid = htonl(current_status.bid);
                    current_status.ask = htonl(current_status.ask);
                    current_status.last = htonl(current_status.last);
                    current_status.orderid = htonl(current_status.orderid);
                    current_status.quantity = htonl(current_status.quantity);



                    response_header.size = htons((uint16_t) sizeof(BRS_STATUS_INFO));
                    if (fail) {
                        send_nack(fd);
                        break;
                    } else {
                        response_header.type = BRS_ACK_PKT;
                    }
                    proto_send_packet(fd, &response_header, &current_status);
                }
                    break;
                case BRS_ESCROW_PKT: {
                    if (data == NULL) {
                        send_nack(fd);
                        break;
                    }
                    amount = *(quantity_t *) data;
                    if (data != NULL) {
                        free(data);
                    }
                    account_increase_inventory(trader_get_account(current_trader), amount);


                    exchange_get_status(exchange, current_account, &current_status);
                    current_status.balance = htonl(current_status.balance);
                    current_status.inventory = htonl(current_status.inventory);
                    current_status.bid = htonl(current_status.bid);
                    current_status.ask = htonl(current_status.ask);
                    current_status.last = htonl(current_status.last);
                    current_status.orderid = htonl(current_status.orderid);
                    current_status.quantity = htonl(current_status.quantity);

                    response_header.size = htons((uint16_t) sizeof(BRS_STATUS_INFO));
                    response_header.type = BRS_ACK_PKT;
                    proto_send_packet(fd, &response_header, &current_status);
                }
                    break;
                case BRS_RELEASE_PKT: {
                    if (data == NULL) {
                        send_nack(fd);
                        break;
                    }
                    amount = *(quantity_t *) data;
                    if (data != NULL) {
                        free(data);
                    }
                    int fail = account_decrease_inventory(trader_get_account(current_trader), amount);


                    exchange_get_status(exchange, current_account, &current_status);
                    current_status.balance = htonl(current_status.balance);
                    current_status.inventory = htonl(current_status.inventory);
                    current_status.bid = htonl(current_status.bid);
                    current_status.ask = htonl(current_status.ask);
                    current_status.last = htonl(current_status.last);
                    current_status.orderid = htonl(current_status.orderid);
                    current_status.quantity = htonl(current_status.quantity);


                    response_header.size = htons( (uint16_t) sizeof(BRS_STATUS_INFO));
                    if (fail) {
                        send_nack(fd);
                        break;
                    } else {
                        response_header.type = BRS_ACK_PKT;
                    }
                    proto_send_packet(fd, &response_header, &current_status);
                }
                    break;
                case BRS_BUY_PKT: {
                    if (data == NULL) {
                        send_nack(fd);
                        break;
                    }
                    BRS_ORDER_INFO order_ = *(BRS_ORDER_INFO *) data;
                    if (data != NULL) {
                        free(data);
                    }
                    orderid_t o_id = exchange_post_buy(exchange, current_trader, ntohl(order_.quantity), ntohl(order_.price));


                    exchange_get_status(exchange, current_account, &current_status);
                    current_status.balance = htonl(current_status.balance);
                    current_status.inventory = htonl(current_status.inventory);
                    current_status.bid = htonl(current_status.bid);
                    current_status.ask = htonl(current_status.ask);
                    current_status.last = htonl(current_status.last);
                    current_status.orderid = htonl(o_id);
                    current_status.quantity = htonl(current_status.quantity);


                    response_header.size = htons((uint16_t) sizeof(BRS_STATUS_INFO));
                    if (!o_id) {
                        send_nack(fd);
                        break;
                    } else {
                        response_header.type = BRS_ACK_PKT;
                    }
                    proto_send_packet(fd, &response_header, &current_status);
                }

                    break;
                case BRS_SELL_PKT: {
                    if (data == NULL) {
                        send_nack(fd);
                        break;
                    }
                    BRS_ORDER_INFO order_ = *(BRS_ORDER_INFO *) data;
                    if (data != NULL) {
                        free(data);
                    }
                    orderid_t o_id = exchange_post_sell(exchange, current_trader, ntohl(order_.quantity), ntohl(order_.price));

                    exchange_get_status(exchange, current_account, &current_status);
                    current_status.balance = htonl(current_status.balance);
                    current_status.inventory = htonl(current_status.inventory);
                    current_status.bid = htonl(current_status.bid);
                    current_status.ask = htonl(current_status.ask);
                    current_status.last = htonl(current_status.last);
                    current_status.orderid = htonl(o_id);
                    current_status.quantity = htonl(current_status.quantity);


                    response_header.size = htons((uint16_t) sizeof(BRS_STATUS_INFO));
                    if (!o_id) {
                        send_nack(fd);
                        break;
                    } else {
                        response_header.type = BRS_ACK_PKT;
                    }
                    proto_send_packet(fd, &response_header, &current_status);
                }

                    break;
                case BRS_CANCEL_PKT: {
                    if (data == NULL) {
                        send_nack(fd);
                        break;
                    }
                    orderid_t order_id = ntohl(*(orderid_t *) data);
                    if (data != NULL) {
                        free(data);
                    }
                    quantity_t order_amount = 0;
                    int fail = exchange_cancel(exchange, current_trader, order_id, &order_amount);

                    exchange_get_status(exchange, current_account, &current_status);
                    current_status.balance = htonl(current_status.balance);
                    current_status.inventory = htonl(current_status.inventory);
                    current_status.bid = htonl(current_status.bid);
                    current_status.ask = htonl(current_status.ask);
                    current_status.last = htonl(current_status.last);
                    current_status.orderid = htonl(order_id);
                    current_status.quantity = htonl(order_amount);


                    response_header.size = htons((uint16_t) sizeof(BRS_STATUS_INFO));
                    if (fail) {
                        send_nack(fd);
                        break;
                    } else {
                        response_header.type = BRS_ACK_PKT;
                    }
                    proto_send_packet(fd, &response_header, &current_status);
                }
                    break;
                case BRS_LOGIN_PKT:
                    if(current_trader != NULL || data == NULL) {
                        send_nack(fd);
                    } else {
                        int name_length = ntohs(header.size);
                        char name[name_length+1];
                        memcpy(name,data,name_length);
                        free(data);
                        name[name_length] = '\0';
                        current_trader = trader_login(fd, name);
                        if(current_trader == NULL) {
                            send_nack(fd);
                        } else {
                            send_ack(fd);
                        }
                    }
                    break;
                default:

                    break;
            }
        }

    }
}