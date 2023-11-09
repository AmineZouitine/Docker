#include "request_informations.h"

#include <stdlib.h>

void free_request_data(struct request_informations *request_informations)
{
    free(request_informations->request_datas.token);
    free(request_informations->request_datas.mediatype);
    free(request_informations->request_datas.mnfst_dgst);
    free(request_informations->request_datas.cnf_digest);
    free(request_informations->request_datas.layer_digest);
    free(request_informations->headers.authorization);
    free(request_informations->headers.accept);
}
