#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	FILE *fp = NULL, *fp_tmp = NULL;
	char file_path[64] = "zcfg_fe_tr98_handle.h", file_tmp_path[64] = "zcfg_fe_tr98_handle_tmp.h";
	char buf[512] = "";
	char *pos = NULL;

	fp     = fopen(file_path, "r");
	fp_tmp = fopen(file_tmp_path, "w");
	if(fp != NULL && fp_tmp != NULL) {
		while(fgets(buf, sizeof(buf), fp) != NULL) {
			if(strstr(buf, "TR98_") != NULL) {
				pos = strchr(buf, ',');
				if(pos != NULL) {
					*pos = '}';
					*(pos + 1) = ',';
					*(pos + 2) = '\0';

					fprintf(fp_tmp, "%s\n", buf);
					continue;
				}
			}
			else if(strstr(buf, "{NULL") != NULL) {
				pos = strchr(buf, ',');
				if(pos != NULL) {
					*pos = '}';
					*(pos + 1) = '\0';
					fprintf(fp_tmp, "%s\n", buf);
					continue;
				}
			}

			fprintf(fp_tmp, "%s", buf);
		}

		fclose(fp);
		fclose(fp_tmp);
	}

	return 0;
}
