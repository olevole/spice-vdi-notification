#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <libwebsockets.h>

#define KGRN "\033[0;32;32m"
#define KCYN "\033[0;36m"
#define KRED "\033[0;32;31m"
#define KYEL "\033[1;33m"
#define KMAG "\033[0;35m"
#define KBLU "\033[0;32;34m"
#define KCYN_L "\033[1;36m"
#define RESET "\033[0m"

char *train[8096];

int id=0;

#define MAXNAME 1024

#define CREATE(result, type, number)  do {\
        if (!((result) = (type *) calloc ((number), sizeof(type))))\
        { perror("malloc failure"); abort(); } } while(0)

struct item_data {
	int id;
	int status;
	char name[MAXNAME];
	int group;
	struct item_data *next;
};

struct item_data *item_list = NULL;

static int destroy_flag = 0;

static void INT_HANDLER(int signo) {
    destroy_flag = 1;
}

int load_data();

int websocket_write_back(struct lws *wsi_in, char *str, int str_size_in) 
{
    if (str == NULL || wsi_in == NULL)
        return -1;

    int n;
    int len;
    char *out = NULL;

    if (str_size_in < 1) 
        len = strlen(str);
    else
        len = str_size_in;

    out = (char *)malloc(sizeof(char)*(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));
    memcpy (out + LWS_SEND_BUFFER_PRE_PADDING, str, len );

    n = lws_write(wsi_in, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);
    printf(KBLU"[websocket_write_back] %s\n"RESET, str);
    free(out);

    return n;
}


static int ws_service_callback(
                         struct lws *wsi,
                         enum lws_callback_reasons reason, void *user,
                         void *in, size_t len)
{

    switch (reason) {

        case LWS_CALLBACK_ESTABLISHED:
            printf(KYEL"[Main Service] Connection established\n"RESET);
            websocket_write_back(wsi ,train, -1);
            break;

        case LWS_CALLBACK_RECEIVE:
            printf(KCYN_L"[Main Service] Server recvived:%s\n"RESET,(char *)in);

            websocket_write_back(wsi ,(char *)in, -1);

            break;
    case LWS_CALLBACK_CLOSED:
            printf(KYEL"[Main Service] Client close.\n"RESET);
        break;

    default:
            break;
    }

    return 0;
}

struct per_session_data {
    int fd;
};

load_data()
{
	struct item_data *newd;
	struct item_data *temp;
	struct item_data *target = NULL, *ch, *next_ch;
	char name[100];
	char tmp[1024];
	int i=0;
	int first=0;

	for (i=0;i<10;i++) {
	CREATE(newd, struct item_data, 1);
	memset(name,0,sizeof(name));
	sprintf(name,"name%d",id);
	newd->id=id;
	newd->status=0;
	newd->group = 1;
	newd->next = item_list;
	item_list = newd;
	strcpy(newd->name,name);
	printf("new thin server [%d][%s] has beed added\n",id,newd->name);
	id++;
	}

	for (i=0;i<15;i++) {
	CREATE(newd, struct item_data, 1);
	memset(name,0,sizeof(name));
	sprintf(name,"gname%d",id);
	newd->id=id;
	newd->status=0;
	newd->group = 2;
	newd->next = item_list;
	if (i==1)
		newd->status=2;
	if (i==3)
		newd->status=1;
	if (i==4)
		newd->status=1;
	if (i==8)
		newd->status=2;
	item_list = newd;
	strcpy(newd->name,name);
	printf("new thin server [%d][%s] has beed added\n",id,newd->name);
	id++;
	}

	strcpy(train,"[ ");
	for (ch = item_list; ch; ch = ch->next) {
		memset(tmp,0,sizeof(tmp));
		if (first==0) {
			sprintf(tmp,"{ \"id\":%d, \"status\":%d, \"name\":\"%s\", \"group\":%d }",ch->id,ch->status,ch->name,ch->group);
		} else {
			sprintf(tmp,",{ \"id\":%d, \"status\":%d, \"name\":\"%s\", \"group\":%d }",ch->id,ch->status,ch->name,ch->group);
		}
		strcat(train,tmp);
		first=1;
	}
	strcat(train,"]");
	printf("%s\n",train);
}

int main(void) {
    int port = 5000;
    const char *interface = NULL;
    struct lws_context_creation_info info;
    struct lws_protocols protocol;
    struct lws_context *context;
    // Not using ssl
    const char *cert_path = NULL;
    const char *key_path = NULL;
    // no special options
    int opts = 0;

    memset(train,0,sizeof(train));
    load_data();

    //* register the signal SIGINT handler */
    struct sigaction act;
    act.sa_handler = INT_HANDLER;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction( SIGINT, &act, 0);

    //* setup websocket protocol */
    protocol.name = "my-echo-protocol";
    protocol.callback = ws_service_callback;
    protocol.per_session_data_size=sizeof(struct per_session_data);
    protocol.rx_buffer_size = 0;

    //* setup websocket context info*/
    memset(&info, 0, sizeof info);
    info.port = port;
    info.iface = interface;
    info.protocols = &protocol;
    info.extensions = lws_get_internal_extensions();
    info.ssl_cert_filepath = cert_path;
    info.ssl_private_key_filepath = key_path;
    info.gid = -1;
    info.uid = -1;
    info.options = opts;

    //* create libwebsocket context. */
    context = lws_create_context(&info);
    if (context == NULL) {
        printf(KRED"[Main] Websocket context create error.\n"RESET);
        return -1;
    }

    printf(KGRN"[Main] Websocket context create success.\n"RESET);

    //* websocket service */
    while ( !destroy_flag ) {
        lws_service(context, 50);
    }
    usleep(10);
    lws_context_destroy(context);

    return 0;
}

