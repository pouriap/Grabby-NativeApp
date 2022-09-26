#pragma once

//TODO: make types integer?

//general defines
#define GRB_ADDON_ID "grabby.pouriap"
#define CMD_MAX_LEN 4096

//message types
#define MSGTYP_GET_AVAIL_DMS "get_available_dms"
#define MSGTYP_AVAIL_DMS "available_dms"
#define MSGTYP_DOWNLOAD "download"
#define MSGTYP_YTDL_INFO "ytdl_getinfo"
#define MSGTYP_YTDL_GET "ytdl_get"
#define MSGTYP_YTDLPROG "app_download_progress"
#define MSGTYP_ERR "app_error"
#define MSGTYP_MSG "app_message"
#define MSGTYP_YTDL_COMP "ytdl_comp"
#define MSGTYP_YTDL_FAIL "ytdl_fail"
#define MSGTYP_UNSUPP "unsupported"