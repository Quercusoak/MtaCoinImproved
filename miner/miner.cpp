#include "../utils/utils.h"

#define MINER_PATH "/mnt/mta/miner_pipe_"

void sendMinerSubscription(int server_fd, const string& miner_pipe_name);
void sendBlockData(int server_fd, const BLOCK_T& block);
void mineBlock(BLOCK_T& block, int miner_fd);

int miner_id;

int main() {
    // freopen( LOG_PATH, "a+", stdout );

    string pipe_name;
    int miner_fd = -1;
    miner_id = 0;
    BLOCK_T block;

    // a)   Open a pipe whose name shall contain the miner id (the following vacant number)
    for(;;) {
        miner_id++;
        pipe_name = MINER_PATH + std::to_string(miner_id);
        
        // Check if the named pipe already exists
        if (access(pipe_name.c_str(), F_OK) == -1) {
            
            if (mkfifo(pipe_name.c_str(), 0666) == -1 && errno != EEXIST) {
                cout << "Failed to create miner pipe: " << strerror(errno) << endl;
                return 1;
            }else{
                break;
            }
        }
    }


    // b)   Send a subscription request with the queue name to the server’s pipe
    int server_fd = open(SERVER_PIPE, O_WRONLY);
    if (server_fd == -1) {
        printf("Failed to open pipe for reading.");
        return 1;
    }

    printf("Miner %d sent connect request to server\n",miner_id);   
    sendMinerSubscription(server_fd, pipe_name);

    //c)	Wait on the pipe to receive the first block.
    miner_fd = open(pipe_name.c_str(), O_RDONLY );

    srand(time(NULL));

    // d)	Start mining
    for (;;){

        if (read(miner_fd, &block, sizeof(BLOCK_T)) > 0){
        
            printf("Miner #%d recieved block: ",miner_id);
            printBlockInfo(block);

            mineBlock(block, miner_fd); 
                
            printf("Miner #%d: mined new block #%d with hash: 0x%X and difficulty: %d\n", miner_id, block.height, block.hash, block.difficulty);
            // printBlockInfo(block);

            sendBlockData(server_fd, block);
            
            //makes pipe blocking and waites for next block
            fcntl(miner_fd, F_SETFL,fcntl(miner_fd, F_GETFL) & ~O_NONBLOCK);
        }
    }

    close(miner_fd);
    close(server_fd);
    fclose(stdout);

    return 0;
}

void sendMinerSubscription(int server_fd, const string& miner_pipe_name) {
    int msg_type = MSG_TYPE_MINER_SUBSCRIPTION;
    write(server_fd, &msg_type, sizeof(msg_type));
    write(server_fd, miner_pipe_name.c_str(), miner_pipe_name.size()); 
}

void sendBlockData(int server_fd, const BLOCK_T& block) {
    int msg_type = MSG_TYPE_BLOCK_DATA;
    write(server_fd, &msg_type, sizeof(msg_type));
    write(server_fd, &block, sizeof(block));
}

void mineBlock(BLOCK_T& block, int miner_fd){ 

    // make pipe non blocking to check if there is a new block
    fcntl(miner_fd, F_SETFL, fcntl(miner_fd, F_GETFL) | O_NONBLOCK);
    
    BLOCK_T new_block;

    bool isMined = false;
    bool isBlockOld = false;

    do {
        isBlockOld = false;
        block.relayed_by = miner_id;
        block.nonce = rand();
        block.timestamp = (int)time(nullptr);

        block.hash = crc32Hash(block);

        while (__builtin_clz(block.hash) != block.difficulty){

            if (read(miner_fd, &new_block, sizeof(BLOCK_T)) > 0){

                if (new_block.height != block.height){
                    block = new_block;
                    isBlockOld = true;
                    break;
                }  
            }
            block.nonce++;
            block.hash = crc32Hash(block);
        }

        if (!isBlockOld){
            isMined = true;
        }

    } while (!isMined);
}