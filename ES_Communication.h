/*
 * ===============================================================
 *                      ES_Communication.h
 * ===============================================================
 *
 * Autor: Julio Cesar S. Fernandes
 * Última Atualização: 10-nov-2024
 * Versão: 1.0
 *
 * ESP-IDF:
 * -
 *
 * Target/Board
 * - Estimulador Elétrico **Nome especifico?**
 *
 * Descrição:
 * - Biblioteca criada para permitir a comunicação UDP e (Wifi)
 * com o estimulador elétrico.
 *
 * Dependências:
 * -
 *
 * Instruções de Uso:
 * - Herdar a classe ES_Communication para reimplementação de metodos
 * e uso generalizado no codigo como é apresentado na linha 57 desde arquivo
 *
 * Histórico de Versões:
 * - Versão 1.0 (20-out-2024): Implementação inicial.
 */

#ifndef ES_COMMUNICATION_H
#define ES_COMMUNICATION_H

#include <string>
#include <winsock2.h> // Permite a comunicação UDP
#include <cstdint>  // Adicione isso para uint16_t

// Enum para Niveis de Log
enum LogType {
    ERRO,
    WARN,
    INFO,
    DEBUG,
    TESTE
};

// Definindo constantes para cores para prints
#define RESET                    "\x1b[0m"
#define RED                      "\x1b[31m"
#define GREEN                    "\x1b[32m"
#define YELLOW                   "\x1b[33m"
#define BLUE                     "\x1b[34m"
#define MAGENTA                  "\x1b[35m"
#define CYAN                     "\x1b[36m"

class ES_Communication {
    // a declaração dessa classe com metodos virtuais mostra que é esperado que classes
    // herdeiras sobrescrevam esses metodos para um uso indiscriminado no programa principal
    // Ex: comunicacao_exemplo.cpp
    // ES_Communication *comm = __nullptr;
    // if(comunicacao_desejada == UDP);
    //      *comm = ES_UdpClient();
    // else // comunicacao_desejada == Wifi
    //      *comm = ES_WifiClient();
    // comm->esConfig();
    // comm->esStart();
    // (...)
    // comm->esSendRecievePackage(...); repare que o ponteiro da classe origial dispensa a necessidade
    //                                  de alterar o codigo independente do tipo de comunicação usado
    //                                  (herança e poliomorfismo)

public:
    virtual ~ES_Communication() = default;
    virtual bool esConfig               () = 0;
    virtual bool esStart                () = 0;
    virtual void esStop                 () = 0;
    virtual int esSendData              (byte *data, unsigned int size_data) = 0;
    virtual int esReceiveData           (char *data_out) = 0;
    virtual int esSendPackage           (byte id, byte *data_in, unsigned int size_data_in) = 0;
    virtual int esSendReceivePackage    (byte id, byte *data_in, unsigned int size_data_in, char *data_out) = 0;
    virtual int esSendCmd               (byte cmd, char *data_out) = 0;
    virtual int esSendConfig            (byte cmd, byte* data_in, unsigned int size_data_in) = 0;
    virtual int esGetNetworkConfig      (/*out*/  byte *ip, /*out*/  byte *mask, /*out*/  byte *getway, /*out*/  unsigned int *port) = 0;
    virtual int esSendSegmentStimulus   (byte *data_in, unsigned int size_data_in, char *data_out) = 0;
    virtual int esSendStimulus          (byte* signal, unsigned int cycles_repetitions, unsigned int samples_in_one_cycle_signal) = 0;
    virtual void esSendFinishTransmission() = 0;
    virtual void SetLogLevel            (LogType level = INFO) = 0;
    virtual void Log                    (LogType level, const std::string& message) = 0;
};

#endif // ES_COMMUNICATION_H
