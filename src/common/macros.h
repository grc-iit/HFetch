//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_MACROS_H
#define HFETCH_MACROS_H
/* Macros */
#define WAIT_UNTILL_SUCESS_BIP(ARG) while(true){\
try {\
ARG;\
break;\
}catch (bip::interprocess_exception &ipce){\
if (ipce.get_error_code() != bip::not_found_error){\
throw ipce;\
}\
}\
}
#define CONF Singleton<ConfigurationManager>::GetInstance()
#endif //HFETCH_MACROS_H
