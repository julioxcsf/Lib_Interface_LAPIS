#include "ES_UdpClient.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <vector>
#include <cmath>
#include <cstdint>  // Para uint8_t e uint16_t
#include <cstring> // Para memcpy, strlen


// construtor da classe que set valores iniciais
ES_UdpClient::ES_UdpClient() : udp_socket(INVALID_SOCKET), running(false) {

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        Log(ERRO, "WSAStartup failed: " + std::to_string(result));
        exit(1);
    }
}

// destrutor encerra a comunicação
ES_UdpClient::~ES_UdpClient() {
    esStop();
    WSACleanup();
}

// configura o alvo para que ocorra a comunicação
bool ES_UdpClient::esConfig() {
    std::string message;

    message = "IP defined: " + std::string(ES_IP);
    Log(INFO, message);

    message = "PORT defined: " + std::to_string(ES_PORT);
    Log(INFO, message);

    message = "TIMEOUT defined: " + std::to_string(ES_TIME_OUT);
    Log(INFO, message);

    memset(&hardwareIPAddr, 0, sizeof(hardwareIPAddr));
    hardwareIPAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ES_IP, &hardwareIPAddr.sin_addr);
    hardwareIPAddr.sin_port = htons(ES_PORT);
    //talvez seja necessário avaliar se existe erro nessas funções para
    //nao retornar sempre true.

    return true;
}

// tenta estruturar o ambiente para a comunicação usando o alvo definido em esConfig()
bool ES_UdpClient::esStart() {
    if (running) {
        Log(WARN, "Already started");
        return false;
    }

    udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_socket == INVALID_SOCKET) {
        Log(ERRO, "Socket creation failed: " + std::to_string(WSAGetLastError()));
        return false;
    }

    DWORD timeout = ES_TIME_OUT;
    if (setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        Log(ERRO, "Failed to set socket receive timeout: " + std::to_string(WSAGetLastError()));
        return false;
    }

    sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons(ES_PORT);

    if (bind(udp_socket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
        Log(ERRO, "Bind failed: " + std::to_string(WSAGetLastError()));
        return false;
    }

    running = true;
    Log(INFO, "Communication started successfully");
    return true;
}

// encerra a comunicação
void ES_UdpClient::esStop() {
    if (running) {
        if (udp_socket != INVALID_SOCKET) {
            closesocket(udp_socket);
            Log(INFO, "Socket closed.");
            udp_socket = INVALID_SOCKET;
            running = false;
        }
    }
}

// função primitiva de envio de bytes
int ES_UdpClient::esSendData(byte *data, unsigned int size_data) {
    if (!running) {
        Log(WARN, "Not started");
        return COMM_NOT_STARTED;
    }

    std::cout << " Dados a enviar: {"; 
    for (int i = 0; i < (int)size_data; i++) {
        if (i == (int)size_data - 1) { std::cout << (int)data[i]; }
        else { std::cout << (int)data[i] << ", "; }
    }
    std::cout << "}" << std::endl;

    if (sendto(udp_socket, (const char*)data, size_data, 0, (sockaddr*)&hardwareIPAddr, sizeof(hardwareIPAddr)) < 0){
        Log(ERRO, "Send failed: " + std::to_string(WSAGetLastError()));
        return COMM_SEND_DATA_ERROR;
    }

    return OK;
}

// função primitiva de recebimento de bytes
int ES_UdpClient::esReceiveData(char *data_out) {
    if (!running) {
        Log(WARN, "Not started");
        return COMM_NOT_STARTED;
    }

    int serverAddrSize = sizeof(hardwareIPAddr);
    int size_data_out = recvfrom(udp_socket, data_out, RECIBE_BUFFER_SIZE, 0, (sockaddr*)&hardwareIPAddr, &serverAddrSize);
    if (size_data_out == SOCKET_ERROR) {
        Log(ERRO, "Receive failed: " + std::to_string(WSAGetLastError()));
        return COMM_RECIVE_DATA_ERROR;
    }

    std::cout << " Dados recebidos: {"; 
    for (int i = 0; i < size_data_out; i++) {
        if (i == (int)size_data_out - 1) { std::cout << (int)data_out[i]; }
        else { std::cout << (int)data_out[i] << ", "; }
    }
    std::cout << "}" << std::endl;
    
    return OK;
}

// função para envio de pacotes
// difere da esRecieveData por ter um enviar um comando antes dos dados no pacote
int ES_UdpClient::esSendPackage(byte id, byte *data_in_ptr, unsigned int size_data) {
    Log(DEBUG, "Dentro da esEnviaPacote");

    byte* datos = new byte[size_data + 5];
    datos[0] = (byte)ES_CMD_P_START;
    datos[1] = (byte)((size_data + 5) & MAX_VALUE);
    datos[2] = (byte)((size_data + 5) >> 8 & MAX_VALUE);
    datos[3] = id;

    if (data_in_ptr != NULL) {
        for (unsigned int i = 0; i < size_data; i++)
            datos[i + 4] = data_in_ptr[i];
    }

    datos[size_data + 4] = (byte)0;
    for (unsigned int i = 1; i < size_data + 4; i++)
        datos[size_data + 4] += datos[i];

    esSendData(datos, size_data + 5);
    delete[] datos;
    return OK;
}

// função de envio e recebimento de pacotes
int ES_UdpClient::esSendReceivePackage(byte id, byte *data_in, unsigned int size_data_in, char *data_out) {

    int resultado = esSendPackage(id, data_in, size_data_in);
    if ( resultado != OK) {
        return resultado;
    }

    return esReceiveData(data_out);
}


int ES_UdpClient::esSendCmd(byte cmd, char *data_out) {
    int resultado = esSendReceivePackage(cmd, nullptr, 0, data_out);
    if (resultado != OK) {
        return resultado;
    }

    // Verificação de resposta válida
    int data_out_length = strlen(data_out);  // Assumindo que data_out é uma string null-terminated
    if (data_out_length >= 5 && (byte)data_out[3] == cmd) {
        return OK;
    } else {
        return COMM_SEND_CMD_ERROR;
    }
}

int ES_UdpClient::esSendConfig(byte cmd, byte* data_in, unsigned int size_data_in) {
    char data_out[RECIBE_BUFFER_SIZE];
    if (this->esSendReceivePackage(cmd, data_in, size_data_in, data_out) != OK) {
        return COMM_SEND_CONFIG_ERROR;
    }
    // Verificação de resposta válida
    int data_out_length = strlen(data_out);  // Assumindo que data_out é uma string null-terminated
    if (data_out_length >= 5 && (byte)data_out[3] == cmd) {
        return OK;
    }
    else {
        return COMM_SEND_CMD_ERROR;
    }
}

// recebe as configurações de rede do dispositivo estimulador
// função usada para confirmar que a conexão com o dispositivo foi bem-sucedida.
int ES_UdpClient::esGetNetworkConfig(/*out*/  byte *ip, /*out*/  byte *mask, /*out*/  byte *getway, /*out*/  unsigned int *port) {
    char data_out[RECIBE_BUFFER_SIZE];
    int recebido = esSendReceivePackage(ES_CMD_GET_NETWORK, nullptr, 0, data_out);
    if (recebido != OK)
    {
        return recebido;
    }
    ip[0] = data_out[4];
    ip[1] = data_out[5];
    ip[2] = data_out[6];
    ip[3] = data_out[7];
    mask[0] = data_out[8];
    mask[1] = data_out[9];
    mask[2] = data_out[10];
    mask[3] = data_out[11];
    getway[0] = data_out[12];
    getway[1] = data_out[13];
    getway[2] = data_out[14];
    getway[3] = data_out[15];
    *port = (unsigned char)data_out[16] | (unsigned char)data_out[17] << 8;
    return OK;
    }

int ES_UdpClient::esSendSegmentStimulus(byte *data_in, unsigned int size_data_in, char *data_out) {
    int resultado;

    do {
        resultado = esSendReceivePackage(ES_CMD_RECIVE_PACKAGE, data_in, size_data_in, data_out);
    } while (resultado != OK || strlen(data_out) < 5 || (byte)data_out[3] != (byte)25);

    return (int)data_out[4];
}

//depois que a função na interface estiver funcional, é necessário passá-la para este arquivo!!(04/11/2024)
int ES_UdpClient::esSendStimulus(byte* signal, unsigned int cycles_repetitions, unsigned int samples_in_one_cycle_signal) {
    try {
        for (unsigned int j = 0; j <= cycles_repetitions; ++j) {
            for (unsigned int i = 0; i <= (unsigned int)(samples_in_one_cycle_signal / 16); ++i) {
                // Extrair o pacote atual (128 bytes)
                byte current_package[16];
                std::memcpy(current_package, &signal[i * 16], 16);

                // Inserir o índice `i` no início do pacote
                byte package_to_send[17];
                package_to_send[0] = i;
                std::memcpy(&package_to_send[1], current_package, 16);

                // Enviar o pacote
                char data_out[RECIBE_BUFFER_SIZE];
                int resultado = esSendReceivePackage(ES_CMD_RECIVE_PACKAGE, package_to_send, 17, data_out);

                // Verificar a resposta
                if (resultado != OK || std::strlen(data_out) < 5 || (byte)data_out[3] != (byte)25) {
                    std::cerr << "Erro ao enviar pacote " << i << " na repeticao " << j << std::endl;
                    std::cout << "Resultado: " << resultado << " | len(data_out): " << std::strlen(data_out) << " | data_out[3]: " << (int)data_out[3] << std::endl;
                    return 1;  // Erro na transmissão
                }
            }
            std::cout << "Repeticao: " << j << " concluida." << std::endl;
        }

        // Finalizar a transmissão
        esSendFinishTransmission();
        return 0;  // Sucesso
    } catch (const std::exception& e) {
        std::cerr << "Ocorreu uma excecao: " << e.what() << std::endl;
        return 1;  // Exceção capturada
    }
}

int ES_UdpClient::esSendStimulus(byte* signal, unsigned int N_packages_16b) { //sobrecarga de func. | essa é a CORRETA!!!
    std::cout << "entrei" << std::endl;
    byte* package_to_send = new byte[257];
    for (unsigned int i = 0; i < N_packages_16b; ++i) {
        std::cout << "i = " << i << std::endl;
        // Extrair o pacote atual (256 bytes ou 128 palavras de 16 bits)
        /*byte current_package[256] = {0};
        std::memcpy(current_package, &signal[i * 256], 256);*/

        // Inserir o índice `i` no início do pacote
        package_to_send = { 0 };// zerando tudo
        package_to_send[0] = i;
        std::memcpy(&package_to_send[1], &signal[i * 256], 256);

        // Enviar o pacote
        char data_out[RECIBE_BUFFER_SIZE];
        int resultado = esSendReceivePackage(ES_CMD_RECIVE_PACKAGE, package_to_send, 257, data_out);

        // Verificar a resposta
        if (resultado != OK || std::strlen(data_out) < 5 || (byte)data_out[3] != (byte)25) {
            std::cerr << "Erro ao enviar pacote " << i << std::endl;
            std::cout << "Resultado: " << resultado << " | len(data_out): " << std::strlen(data_out) << " | data_out[3]: " << (int)data_out[3] << std::endl;
            return 1;  // Erro na transmissão
        }
    }
    delete[] package_to_send;
    // Finalizar a transmissão
    esSendFinishTransmission();
    return 0;  // Sucesso
}

void ES_UdpClient::esSendFinishTransmission() {
    // Implementar a função para enviar o comando de finalização
    // por exemplo:
    byte end_signal = ES_CMD_END_TRANSMISION;
    char data_out[RECIBE_BUFFER_SIZE];
    esSendReceivePackage(ES_CMD_END_TRANSMISION, &end_signal, 1, data_out);
}

// função que define qual é o nivel de interesse para os prints de DEBUG
void ES_UdpClient::SetLogLevel(LogType level) {
    logLevel = level;
}

// função para printar o que é desejado variando a cor do logLevel dependendo deste
void ES_UdpClient::Log(LogType level, const std::string& message) {
    if (level <= logLevel) {
        std::string color, stringlog;
        switch (level) {
        case DEBUG: 
            color = CYAN;
            stringlog = "DEBUG: ";
            break;
        case INFO: 
            color = GREEN;
            stringlog = "INFO: ";
            break;
        case WARN: 
            color = YELLOW; 
            stringlog = "WARN: ";
            break;
        case ERRO: 
            color = RED; 
            stringlog = "ERRO: ";
            break;
        case TESTE: 
            color = MAGENTA; 
            stringlog = "TESTE: ";
            break;
        }
        std::cout << color << stringlog << RESET << message << std::endl;
    }
}
