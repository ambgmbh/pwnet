/* Copyright (C), 2007 by Stephen Hurd */

#include <stdio.h>
#include <string.h>

#include "gen_defs.h"
#include "ini_file.h"

#include "ciolib.h"
#include "allfonts.h"

#include "uifc.h"
#include "filepick.h"

#include "bbslist.h"
#include "fonts.h"
#include "syncterm.h"
#include "uifcinit.h"

void free_font_files(struct font_files *ff)
{
	int	i;

	if(ff==NULL)
		return;
	for(i=0; ff[i].name != NULL; i++) {
		FREE_AND_NULL(ff[i].name);
		FREE_AND_NULL(ff[i].path8x8);
		FREE_AND_NULL(ff[i].path8x14);
		FREE_AND_NULL(ff[i].path8x16);
	}
	free(ff);
}

void save_font_files(struct font_files *fonts)
{
	FILE	*inifile;
	char	inipath[MAX_PATH+1];
	char	newfont[MAX_PATH+1];
	char	*fontid;
	str_list_t	ini_file;
	str_list_t	fontnames;
	int		i;

	if(safe_mode)
		return;
	get_syncterm_filename(inipath, sizeof(inipath), SYNCTERM_PATH_INI, FALSE);
	if((inifile=fopen(inipath,"r"))!=NULL) {
		ini_file=iniReadFile(inifile);
		fclose(inifile);
	}
	else {
		ini_file=strListInit();
	}

	fontnames=iniGetSectionList(ini_file, "Font:");

	/* TODO: Remove all sections... we don't *NEED* to do this */
	while((fontid=strListPop(&fontnames))!=NULL) {
		iniRemoveSection(&ini_file, fontid);
		free(fontid);
	}

	if(fonts != NULL) {
		for(i=0; fonts[i].name && fonts[i].name[0]; i++) {
			sprintf(newfont,"Font:%s",fonts[i].name);
			if(fonts[i].path8x8)
				iniSetString(&ini_file, newfont, "Path8x8", fonts[i].path8x8, &ini_style);
			if(fonts[i].path8x14)
				iniSetString(&ini_file, newfont, "Path8x14", fonts[i].path8x14, &ini_style);
			if(fonts[i].path8x16)
				iniSetString(&ini_file, newfont, "Path8x16", fonts[i].path8x16, &ini_style);
		}
	}
	if((inifile=fopen(inipath,"w"))!=NULL) {
		iniWriteFile(inifile,ini_file);
		fclose(inifile);
	}
	else {
		uifc.helpbuf="There was an error writing the INI file.\nCheck permissions and try again.\n";
		uifc.msg("Cannot write to the .ini file!");
	}

	strListFree(&fontnames);
	strListFree(&ini_file);
}

struct font_files *read_font_files(int *count)
{
	FILE	*inifile;
	char	inipath[MAX_PATH+1];
	char	fontpath[MAX_PATH+1];
	char	*fontid;
	str_list_t	fonts;
	struct font_files	*ret=NULL;
	struct font_files	*tmp;

	*count=0;
	get_syncterm_filename(inipath, sizeof(inipath), SYNCTERM_PATH_INI, FALSE);
	if((inifile=fopen(inipath, "r"))==NULL) {
		return(ret);
	}
	fonts=iniReadSectionList(inifile, "Font:");
	while((fontid=strListPop(&fonts))!=NULL) {
		if(!fontid[5]) {
			free(fontid);
			continue;
		}
		(*count)++;
		tmp=(struct font_files *)realloc(ret, sizeof(struct font_files)*(*count+1));
		if(tmp==NULL) {
			count--;
			free(fontid);
			continue;
		}
		ret=tmp;
		ret[*count].name=NULL;
		ret[*count-1].name=strdup(fontid+5);
		if((ret[*count-1].path8x8=iniReadString(inifile,fontid,"Path8x8",NULL,fontpath))!=NULL)
			ret[*count-1].path8x8=strdup(fontpath);
		if((ret[*count-1].path8x14=iniReadString(inifile,fontid,"Path8x14","",fontpath))!=NULL)
			ret[*count-1].path8x14=strdup(fontpath);
		if((ret[*count-1].path8x16=iniReadString(inifile,fontid,"Path8x16",NULL,fontpath))!=NULL)
			ret[*count-1].path8x16=strdup(fontpath);
		free(fontid);
	}
	fclose(inifile);
	strListFree(&fonts);
	return(ret);
}

void load_font_files(void)
{
	int count=0;
	int i;
	int	nextfont=CONIO_FIRST_FREE_FONT;
	struct font_files *ff;
	FILE *fontfile;
	char	*fontdata;

	ff=read_font_files(&count);
	for(i=0; i<count; i++) {
		if(conio_fontdata[nextfont].eight_by_sixteen)
			FREE_AND_NULL(conio_fontdata[nextfont].eight_by_sixteen);
		if(conio_fontdata[nextfont].eight_by_fourteen)
			FREE_AND_NULL(conio_fontdata[nextfont].eight_by_fourteen);
		if(conio_fontdata[nextfont].eight_by_sixteen)
			FREE_AND_NULL(conio_fontdata[nextfont].eight_by_eight);
		if(conio_fontdata[nextfont].desc)
			FREE_AND_NULL(conio_fontdata[nextfont].desc);
		if(ff[i].name)
			conio_fontdata[nextfont].desc=strdup(ff[i].name);
		else
			continue;
		if(ff[i].path8x8 && ff[i].path8x8[0]) {
			if((fontfile=fopen(ff[i].path8x8,"r"))!=NULL) {
				if((fontdata=(char *)malloc(2048))!=NULL) {
					if(fread(fontdata, 1, 2048, fontfile)==2048) {
						conio_fontdata[nextfont].eight_by_eight=fontdata;
					}
					else {
						free(fontdata);
					}
				}
			}
		}
		if(ff[i].path8x14 && ff[i].path8x14[0]) {
			if((fontfile=fopen(ff[i].path8x14,"r"))!=NULL) {
				if((fontdata=(char *)malloc(3584))!=NULL) {
					if(fread(fontdata, 1, 3584, fontfile)==3584) {
						conio_fontdata[nextfont].eight_by_fourteen=fontdata;
					}
					else {
						free(fontdata);
					}
				}
			}
		}
		if(ff[i].path8x16 && ff[i].path8x16[0]) {
			if((fontfile=fopen(ff[i].path8x16,"r"))!=NULL) {
				if((fontdata=(char *)malloc(4096))!=NULL) {
					if(fread(fontdata, 1, 4096, fontfile)==4096) {
						conio_fontdata[nextfont].eight_by_sixteen=fontdata;
					}
					else {
						free(fontdata);
					}
				}
			}
		}
		nextfont++;
	}
	free_font_files(ff);

	for(i=0; conio_fontdata[i].desc != NULL; i++) {
		font_names[i]=conio_fontdata[i].desc;
		if(!strcmp(conio_fontdata[i].desc,"Codepage 437 English")) {
			default_font=i;
		}
	}
	font_names[i]="";
}

int	find_font_id(char *name)
{
	int ret=0;
	int i;

	for(i=0; i<256; i++) {
		if(!conio_fontdata[i].desc)
			continue;
		if(!strcmp(conio_fontdata[i].desc,name)) {
			ret=i;
			break;
		}
	}
	return(ret);
}

void font_management(void)
{
	int i,j;
	int cur=0;
	int bar=0;
	int fcur=0;
	int fbar=0;
	int	count=0;
	struct font_files	*fonts;
	char	*opt[256];
	char	opts[5][80];
	struct font_files	*tmp;
	char	str[128];

	fonts=read_font_files(&count);
	opts[4][0]=0;

	for(;;) {
		uifc.helpbuf=	"`Font Management`\n\n"
						"Allows you to add and remove font files to/from the default font set.\n\n"
						"`INS` Adds a new font.\n"
						"`DEL` Removes an existing font.\n\n"
						"Selecting a font allows you to set the files for all three font sizes:\n"
						"8x8, 8x14, and 8x16.";
		if(fonts) {
			for(j=0;fonts[j].name && fonts[j].name[0]; j++)
				opt[j]=fonts[j].name;
			opt[j]="";
		}
		else {
			opts[0][0]=0;
			opt[0]=opts[0];
		}
		i=uifc.list(WIN_SAV|WIN_INS|WIN_INSACT|WIN_DEL|WIN_XTR|WIN_ACT,0,0,0,&cur,&bar,"Font",opt);
		if(i==-1) {
			save_font_files(fonts);
			free_font_files(fonts);
			return;
		}
		for(;;) {
			char 	*fontmask;
			int		show_filepick=0;
			char	**path;

			if(i&MSK_DEL) {
				FREE_AND_NULL(fonts[cur].name);
				FREE_AND_NULL(fonts[cur].path8x8);
				FREE_AND_NULL(fonts[cur].path8x14);
				FREE_AND_NULL(fonts[cur].path8x16);
				memmove(&(fonts[cur]),&(fonts[cur+1]),sizeof(struct font_files)*(count-cur-1));
				count--;
				break;
			}
			if(i&MSK_INS) {
				str[0]=0;
				uifc.helpbuf="Enter the name of the font as you want it to appear\nin menus.";
				if(uifc.input(WIN_SAV|WIN_MID,0,0,"Font Name",str,50,0)==-1)
					break;
				count++;
				tmp=(struct font_files *)realloc(fonts, sizeof(struct font_files)*(count+1));
				if(tmp==NULL) {
					uifc.msg("realloc() failure, cannot add font.");
					count--;
					break;
				}
				fonts=tmp;
				memmove(fonts+cur+1,fonts+cur,sizeof(struct font_files)*(count-cur));
				memset(&(fonts[count]),0,sizeof(fonts[count]));
				fonts[cur].name=strdup(str);
				fonts[cur].path8x8=NULL;
				fonts[cur].path8x14=NULL;
				fonts[cur].path8x16=NULL;
			}
			for(i=0; i<5; i++)
				opt[i]=opts[i];
			uifc.helpbuf="Font Details\n";
			sprintf(opts[0],"Name: %.50s",fonts[cur].name?fonts[cur].name:"<undefined>");
			sprintf(opts[1],"8x8   %.50s",fonts[cur].path8x8?fonts[cur].path8x8:"<undefined>");
			sprintf(opts[2],"8x14  %.50s",fonts[cur].path8x14?fonts[cur].path8x14:"<undefined>");
			sprintf(opts[3],"8x16  %.50s",fonts[cur].path8x16?fonts[cur].path8x16:"<undefined>");
			opts[4][0]=0;
			i=uifc.list(WIN_SAV|WIN_ACT|WIN_INS|WIN_INSACT|WIN_DEL,0,0,0,&fcur,&fbar,"Font",opt);
			if(i==-1)
				break;
			switch(i) {
				case 0:
					SAFECOPY(str,fonts[cur].name);
					FREE_AND_NULL(fonts[cur].name);
					uifc.helpbuf="Enter the name of the font as you want it to appear\nin menus.";
					uifc.input(WIN_SAV|WIN_MID,0,0,"Font Name",str,50,K_EDIT);
					fonts[cur].name=strdup(str);
					show_filepick=0;
					break;
				case 1:
					sprintf(str,"8x8 %.50s",fonts[cur].name);
					path=&(fonts[cur].path8x8);
					fontmask="*.f8";
					show_filepick=1;
					break;
				case 2:
					sprintf(str,"8x14 %.50s",fonts[cur].name);
					path=&(fonts[cur].path8x14);
					fontmask="*.f14";
					show_filepick=1;
					break;
				case 3:
					sprintf(str,"8x16 %.50s",fonts[cur].name);
					path=&(fonts[cur].path8x16);
					fontmask="*.f16";
					show_filepick=1;
					break;
			}
			if(show_filepick && !safe_mode) {
				int result;
				struct file_pick fpick;
				char	*savbuf;
				struct text_info	ti;

				gettextinfo(&ti);
				savbuf=(char *)alloca((ti.screenheight-2)*ti.screenwidth*2);
				if(savbuf==NULL) {
					uifc.helpbuf="malloc() has failed.  Available Memory is dangerously low.";
					uifc.msg("malloc() failure.");
					continue;
				}
				gettext(1,2,ti.screenwidth,ti.screenheight-1,savbuf);
				result=filepick(&uifc, str, &fpick, ".", fontmask, UIFC_FP_ALLOWENTRY);
				if(result!=-1 && fpick.files>0) {
					FREE_AND_NULL(*path);
					*(path)=strdup(fpick.selected[0]);
				}
				filepick_free(&fpick);
				puttext(1,2,ti.screenwidth,ti.screenheight-1,savbuf);
			}
		}
	}
}
