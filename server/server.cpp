#include <vector>
#include <fstream>
#include "../utils/utils.h"

const char* directory = "/mnt/mta";
const char* config_file_name = "/mnt/mta/mtacoin.config";

int DIFFICULTY;
vector<string> miner_pipes;
vector<BLOCK_T> blockchain;

int getConfigDifficulty();

int openServerPipe();

void runServer(int server_fd);

void subscribeMiner(int server_fd, const BLOCK_T& new_empty_block);

void broadcast_to_miners(const BLOCK_T& new_block) {
    for (const auto& miner_pipe : miner_pipes) {
        int miner_fd = open(miner_pipe.c_str(), O_WRONLY | O_NONBLOCK);
        if (miner_fd != -1) {
            write(miner_fd, &new_block, sizeof(BLOCK_T));
            close(miner_fd);
        }
    }
}


int main(int argc,  char* argv[]) {

    // Initialize logging
    // freopen( LOG_PATH, "a+", stdout );

    DIFFICULTY = getConfigDifficulty();
    
    if (DIFFICULTY ==-1){
        return 1;
    }
    cout << "Difficulty set to: " << DIFFICULTY << "\n";

    int server_fd = openServerPipe();
    if (server_fd == -1) {
        return 1;
    }
   
    runServer(server_fd);

    close(server_fd);
    unlink(SERVER_PIPE);
    fclose(stdout);

    return 0;
}

void runServer(int server_fd){

    int msg_type;
    unsigned int calcHash;

    //Generate an initial genesis block
    BLOCK_T genesisBlock = {0, (int)time(nullptr), 0, 0, DIFFICULTY, 0, -1};
    calculateHashcrc32(genesisBlock, DIFFICULTY);
    blockchain.push_back(genesisBlock);
    printf("Server: Genesis block:");
    printBlockInfo(genesisBlock);

    BLOCK_T new_empty_block = newServerBlockForMining(blockchain.back());

    for (;;){

        if (read(server_fd, &msg_type, sizeof(msg_type)) > 0){

            switch (msg_type){
                case MSG_TYPE_MINER_SUBSCRIPTION:

                    subscribeMiner(server_fd, new_empty_block);
                    break;

                case MSG_TYPE_BLOCK_DATA:

                    BLOCK_T recieved_block;
                    read(server_fd, &recieved_block, sizeof(BLOCK_T));

                    //validate block - verify its proof-of-work, check for leading 0 bits
                    calcHash = crc32Hash(recieved_block);
                    
                    if ( calcHash == recieved_block.hash && (__builtin_clz(recieved_block.hash) == DIFFICULTY)) {
                        
                        if (recieved_block.prev_hash == blockchain.back().hash) {
                            
                            blockchain.push_back(recieved_block);
                            printf("\nServer: new block by #%d: ", recieved_block.relayed_by);
                            printBlockInfo(recieved_block);

                            new_empty_block = newServerBlockForMining(blockchain.back());
                            broadcast_to_miners(new_empty_block);

                        } else {
                            printErrorWrongPrevHash(recieved_block, blockchain.back().prev_hash);
                        }
                    } else {
                        printErrorBadHash(recieved_block, calcHash);
                    }

                    break;
                default:
                    break;
            }  
        }      
    }
}

int getConfigDifficulty(){
    
    string line;
    ifstream config_file(config_file_name);

    if (!config_file.is_open()) {
        std::cerr << "Failed to open configuration file\n";
        return -1;
    }
      
    while( getline(config_file, line) ){
        
        if (line.rfind("DIFFICULTY=", 0) == 0) {
            string difficulty_str = line.substr(11); // Length of "difficulty="
            char* end;
            long difficulty_value = std::strtol(difficulty_str.c_str(), &end, 10);

            // Check if the conversion was successful
            if (*end == '\0') {
                return static_cast<int>(difficulty_value);
            } else {
                std::cerr << "Invalid difficulty value in config file\n";
                return -1;
            }
            break;
        }
    }
    
    return -1;
}

int openServerPipe(){
    
   // Ensure the directory exists 
    if (access(directory, F_OK) != 0) {
        if (mkdir(directory, 0777) == -1) {
            cout << "Failed to create directory: " << strerror(errno) << std::endl;
            return -1;
        }
    }

    // Check if the FIFO already exists
    if (access(SERVER_PIPE, F_OK) == 0) {
        // If it exists, unlink it
        if (unlink(SERVER_PIPE) == -1) {
            cout << "Failed to remove existing FIFO: " << strerror(errno) << std::endl;
            return -1;
        }
    }

    if (mkfifo(SERVER_PIPE, 0666) == -1) {
        cout << "Failed to create named server pipe." << endl;
        return -1;
    }

    // Open the pipe for reading
    int server_fd = open(SERVER_PIPE, O_RDONLY);
    if (server_fd == -1) {
        cout << "Server: Failed to open server pipe" << endl;
        return -1;
    }

    return server_fd;
}

void subscribeMiner(int server_fd, const BLOCK_T& new_empty_block){

    char miner_pipe_name[256];
    ssize_t bytes_read = read(server_fd, miner_pipe_name, sizeof(miner_pipe_name));
                    
    if (bytes_read > 0) {
        string miner_pipe(miner_pipe_name, bytes_read);
        
        // Send current block to new miner
        int miner_fd = open(miner_pipe.c_str(), O_WRONLY);
        if (miner_fd != -1) {

            cout << "\nNew miner subscribed: " << miner_pipe << endl;
            miner_pipes.push_back(miner_pipe);

            ssize_t bytes_written = write(miner_fd, &new_empty_block, sizeof(BLOCK_T));
            
            if (bytes_written == -1) {
                cout << "Failed to write to miner pipe: " << strerror(errno) << std::endl;
            }

            close(miner_fd);
        }else{
            cout << "Failed to open miner pipe." << endl;
        }
    }
}