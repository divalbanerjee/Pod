#include "codec.h"

user_select_ptr codec::decode_input(const std::string & x){
    user_select_ptr ep;
    //ensure that size == 6, and first letters are FSM
    if( (x.size()==6) & (x.substr(0,3)=="FSM")){
        std::string state = x.substr(3,3);
        if(state == "SMD")
             ep = user_select_ptr(new user_select(SAFE_MODE));
        else if(state == "ISN")
             ep = user_select_ptr(new user_select(INIT_SENSORS));
        else if(state == "EMB")
             ep = user_select_ptr(new user_select(E_BRAKE));
        else if(state == "FTA")
             ep = user_select_ptr(new user_select(FUNCT_A));
        else if(state == "FTB")
             ep = user_select_ptr(new user_select(FUNCT_B));
        else if(state == "FTC")
             ep = user_select_ptr(new user_select(FUNCT_C));
        else if(state == "FTD")
             ep = user_select_ptr(new user_select(FUNCT_D));
        else if(state == "LDG")
             ep = user_select_ptr(new user_select(LOADING));
        else if(state == "FA1")
             ep = user_select_ptr(new user_select(FLIGHT_A));
        }
    else {
        ep = user_select_ptr(new user_select(SAFE_MODE));
    }
    return ep;
}


void codec::append_to_data_buffer(std::string & buff, std::string & type, double * data, size_t data_length){
    std::string message;
    
    message.reserve((21 * data_length));
    for(size_t i = 0; i<data_length; i++){
        char tmp[21];
        sprintf(tmp, "%F", data[i]);
        message += tmp;
        message += ",";
    }
    //remove last comma and compute size
    size_t message_size = result.length();
    message = message.substr(0, message_size-1);
    message_size--;

    //calculate message size
	char tmp[5];
	sprintf(tmp,"%04d", message_size);//write message size 


    //reserver extra space to buffer
    buff.reserve(7+message_size+buff.length());
    //add to buffer
    buff+=tmp;
    buff+=type;
    buff+=message;
    
}



