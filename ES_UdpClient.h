/*
 * ===============================================================
 *                      ES_UdpClient.h
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
 * - Implementação da classe especifica para comunicação UDP
 * com o Estimulador Elétrico.
 *
 * Dependências:
 * - "ES_Communication.h"
 *
 * Instruções de Uso:
 * - redeclarar os metodos da classe original para torná-los funcionais
 *
 * Histórico de Versões:
 * - Versão 1.0 (20-out-2024): Implementação inicial.
 */

#ifndef ES_UDPCLIENT_H
#define ES_UDPCLIENT_H

#include "ES_Communication.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>    // Para memcpy, strlen
#include <string>
#include <vector>

// MACROS CODIGOS DE ERROS
#define OK                         1
#define COMM_NOT_STARTED          -1
#define COMM_SEND_DATA_ERROR      -2
#define COMM_RECIVE_DATA_ERROR    -3
#define COMM_SEND_CMD_ERROR       -4
#define COMM_SEND_CONFIG_ERROR    -5

// MACROS DE COMUNICACAO
#define ES_IP                    "192.168.0.40"
#define ES_PORT                  670
#define ES_TIME_OUT              150 // ATENÇÃO - ESSE TIMEOUT SE MOSTROU INSUFICIENTE PARA TRANSMISSÃO DE MUITOS DADOS DE UMA VEZ!
                                     // 100 se demonstrou insuficiente para transmitir 16 bytes e receber a resposta.
                                     // por testes, 150 ja foi suficiente

// MACROS UDP
#define RECIBE_BUFFER_SIZE       4096   // tamanho em bytes do buffer

// Comandos para o Hardware
#define ES_CMD_STATE_CHANNELS           0x04
#define ES_CMD_PACKAGE_NUM              0x06
#define ES_CMD_SAMPLE_NUM               0x07
#define ES_CMD_STIMULATION_MODE         0x08
#define ES_CMD_STIMULUS_TYPE            0x09
#define ES_CMD_SAMPLING_FREQUENCY       0x0A
#define ES_CMD_CYCLES_NUM               0x0B
#define ES_CMD_STIMULUS_INTENSITY       0x0C
#define ES_CMD_CHECK_ELECTRODOS         0x0E
#define ES_CMD_TON_TOFF                 0x10
#define ES_CMD_PROTOCOL                 0x11
#define ES_CMD_PLAY_STIMULUS           0x14
#define ES_CMD_TIME_TRIGG               0x15
#define ES_CMD_STOP_STIMULUS            0x16
#define ES_CMD_RECIVE_PACKAGE           0x19
#define ES_CMD_GET_NETWORK              0x1E
#define ES_CMD_END_TRANSMISION          0x21
#define ES_CMD_RESET_DEVICE             0x2A
#define ES_CMD_P_START                  0x5A


// Valor maximo do Byte: 255
#define MAX_VALUE                0xFF

class ES_UdpClient : public ES_Communication {
    // classe herdeira de ES_Communication que sobrescreve todos os metodos

public:
    ES_UdpClient();
    ~ES_UdpClient();

    bool esConfig            () override;
    bool esStart             () override;
    void esStop              () override;
    int esSendData           (byte *data, unsigned int size_data) override;
    int esReceiveData        (char *data_out) override;
    int esSendPackage        (byte id, byte *data_in, unsigned int size_data_in) override;
    int esSendReceivePackage (byte id, byte *data_in, unsigned int size_data_in, char *data_out) override;
    int esSendCmd            (byte cmd, char *data_out) override;
    int esSendConfig         (byte cmd, byte* data_in, unsigned int size_data_in) override;
    int esGetNetworkConfig   (/*out*/  byte *ip, /*out*/  byte *mask, /*out*/  byte *getway, /*out*/  unsigned int *port) override;
    int esSendSegmentStimulus(byte *data_in, unsigned int size_data_in, char *data_out) override;
    int esSendStimulus       (byte* signal, unsigned int cycles_repetitions, unsigned int samples_in_one_cycle_signal) override;
    int esSendStimulus       (byte* signal, unsigned int N_packages_16b);//nao está no communication.h
    void esSendFinishTransmission() override;

    void SetLogLevel         (LogType level = INFO) override;
    void Log                 (LogType level, const std::string& message) override;

private:
    SOCKET udp_socket;
    sockaddr_in hardwareIPAddr; // IP alvo - o hardware
    bool running;
    LogType logLevel;

    void PrintLog(const std::string& message, 
                  const std::string& color, 
                  const std::string& stringlog);
};

#endif // ES_UDPCLIENT_H
