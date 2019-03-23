//
// Created by hariharan on 3/19/19.
//

#include "file_client.h"

ServerStatus FileClient::Read(PosixFile source, PosixFile destination) {
    std::string file_path=std::string(source.layer.layer_loc.c_str())+FILE_SEPARATOR+std::string(source.filename.c_str());
    FILE* fh = fopen(file_path.c_str(),"r");
    fseek(fh,source.segment.start,SEEK_SET);
    fread(destination.data,source.segment.end-source.segment.start, 1,fh);
    fclose(fh);
    return SERVER_SUCCESS;
}

ServerStatus FileClient::Write(PosixFile source, PosixFile destination) {
    std::string file_path=std::string(destination.layer.layer_loc.c_str())+FILE_SEPARATOR+std::string(destination.filename.c_str());
    FILE* fh = fopen(file_path.c_str(),"r+");
    if(fh==NULL){
        fh = fopen(file_path.c_str(),"w+");
    }
    fseek(fh,destination.segment.start,SEEK_SET);
    fread(source.data,destination.segment.end-destination.segment.start, 1,fh);
    fclose(fh);
    return SERVER_SUCCESS;
}

ServerStatus FileClient::Delete(PosixFile file) {
    std::string file_path=std::string(file.layer.layer_loc.c_str())+FILE_SEPARATOR+std::string(file.filename.c_str());
    FILE* fh = fopen(file_path.c_str(),"r");
    size_t end = fseek(fh,0,SEEK_END);
    if(file.segment.end - file.segment.start + 1 - end == 0){
        remove(file_path.c_str());
        return SERVER_SUCCESS;
    }
    fseek(fh,0,SEEK_SET);
    void* data=malloc(end);
    fread(data,end, 1,fh);
    fclose(fh);
    fh = fopen(file_path.c_str(),"w+");
    if(file.segment.start > 0) fwrite(data, file.segment.start - 1, 1, fh);
    if(file.segment.end < end) fwrite((char*)data + file.segment.end, end - file.segment.end + 1, 1, fh);
    fclose(fh);
    return SERVER_SUCCESS;
}

double FileClient::GetCurrentUsage(Layer l) {
    std::string cmd="du -s "+std::string(l.layer_loc.c_str())+" | awk {'print$1'}";
    FILE *fp;
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return std::stoll(result)-4;
}
