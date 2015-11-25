#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "types.h"
#include "utils.h"

//Build with: gcc -o ctr-logobuilder ctr-logobuilder.c utils.c

typedef struct {
	u8 magicnum[4];//0x54594c43 "CLYT"
	u8 bom[2];
	u8 headerlen[2];
	u8 revision[4];
	u8 filesize[4];
	u8 total_sections[4];
} bclyt_header;

char *darc_dirpath = NULL;
u8 *filebuf;
bclyt_header *layout_header;
u32 filebuf_size = 0x4000, layout_pos = 0;

int layout_addsection(u32 sectionid, u8 *section_payload, u32 section_payloadsize)
{
	if(layout_pos+8+section_payloadsize > filebuf_size)
	{
		printf("This additional layout section is too large.\n");
		return 5;
	}

	putle32(&filebuf[layout_pos+0], sectionid);
	putle32(&filebuf[layout_pos+4], 8+section_payloadsize);
	if(section_payload)memcpy(&filebuf[layout_pos+8], section_payload, section_payloadsize);

	layout_pos+= 8+section_payloadsize;

	putle32(layout_header->total_sections, getle32(layout_header->total_sections) + 1);

	return 0;
}

int layout_addmatsection(char *name)
{
	u32 pos;
	float tmpfloat;
	u32 *floatptr = (u32*)&tmpfloat;
	u8 section_data[0x58];

	if(strlen(name) > 0x13)
	{
		printf("The material name is too long.\n");
		return 6;
	}

	memset(section_data, 0, sizeof(section_data));

	putle32(&section_data[0x0], 1);//total materials
	putle32(&section_data[0x4], 0x10);//Offset to the material entry.
	strncpy((char*)&section_data[0x8], name, 0x13);

	for(pos=0x1c; pos<0x38; pos+=4)//colors
	{
		if(pos==0x1c)
		{
			putle32(&section_data[pos], 0x00ffffff);
		}
		else
		{
			putle32(&section_data[pos], 0xffffffff);
		}
	}

	pos = 0x38;
	putle32(&section_data[pos], 0x15);//flags
	pos+=4;

	putle32(&section_data[pos], 0x04040000);//Texture index + texture mapping.
	pos+=4;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);//Translation x
	pos+=4;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);//Translation y
	pos+=4;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);//Rotation
	pos+=4;

	tmpfloat = 1.0f;
	putle32(&section_data[pos], *floatptr);//Scale x
	pos+=4;

	tmpfloat = 1.0f;
	putle32(&section_data[pos], *floatptr);//Scale y
	pos+=4;

	putle32(&section_data[pos], 0x0);//unknown
	pos+=4;

	return layout_addsection(0x3174616d, section_data, 0x58);
}

int layout_addpansection(char *name, u8 visibility, float width, float height)
{
	int ret=0;
	u32 pos;
	float tmpfloat;
	u32 *floatptr = (u32*)&tmpfloat;
	u8 section_data[0x44];

	if(strlen(name) > 0xf)
	{
		printf("The pane name is too long.\n");
		return 6;
	}

	memset(section_data, 0, sizeof(section_data));

	section_data[0x0] = visibility;//visibility
	section_data[0x1] = 0x4;//origin
	section_data[0x2] = 0xff;//alpha

	strncpy((char*)&section_data[0x4], name, 0xf);
	//Leave the "user_data" at zeros.
	
	pos = 0x1c;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);//xTranslate
	pos+=4;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);//yTranslate
	pos+=4;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);//zTranslate
	pos+=4;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);//xRotate
	pos+=4;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);//yRotate
	pos+=4;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);//zRotate
	pos+=4;

	tmpfloat = 1.0f;
	putle32(&section_data[pos], *floatptr);//xScale
	pos+=4;

	tmpfloat = 1.0f;
	putle32(&section_data[pos], *floatptr);//yScale
	pos+=4;

	tmpfloat = width;
	putle32(&section_data[pos], *floatptr);//width
	pos+=4;

	tmpfloat = height;
	putle32(&section_data[pos], *floatptr);//height
	pos+=4;

	ret = layout_addsection(0x316e6170, section_data, 0x44);
	if(ret)return ret;

	return layout_addsection(0x31736170, NULL, 0);
}

int layout_finishpane()
{
	return layout_addsection(0x31656170, NULL, 0);
}

int layout_addpicsection(char *name, float width, float height, float x, float y, float z)
{
	u32 pos;
	float tmpfloat;
	u32 *floatptr = (u32*)&tmpfloat;
	u8 section_data[0x78];

	if(strlen(name) > 0x17)
	{
		printf("The pane name is too long.\n");
		return 6;
	}

	memset(section_data, 0, sizeof(section_data));

	section_data[0x0] = 0x1;//visibility
	section_data[0x1] = 0x7;//origin
	section_data[0x2] = 0xff;//alpha
	section_data[0x3] = 0x0;//alpha2

	strncpy((char*)&section_data[0x4], name, 0x17);
	
	pos = 0x1c;

	tmpfloat = x;
	putle32(&section_data[pos], *floatptr);//x
	pos+=4;

	tmpfloat = y;
	putle32(&section_data[pos], *floatptr);//y
	pos+=4;

	tmpfloat = z;
	putle32(&section_data[pos], *floatptr);//z
	pos+=4;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);//xFlip
	pos+=4;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);//yFlip
	pos+=4;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);//angle
	pos+=4;

	tmpfloat = 1.0f;
	putle32(&section_data[pos], *floatptr);//xMag
	pos+=4;

	tmpfloat = 1.0f;
	putle32(&section_data[pos], *floatptr);//yMag
	pos+=4;

	tmpfloat = width;
	putle32(&section_data[pos], *floatptr);//width
	pos+=4;

	tmpfloat = height;
	putle32(&section_data[pos], *floatptr);//height
	pos+=4;

	//Unknown data specific to CLYT(which the original Wii format didn't have) starts here.

	putle32(&section_data[pos], 0xffffffff);
	pos+=4;
	putle32(&section_data[pos], 0xffffffff);
	pos+=4;
	putle32(&section_data[pos], 0xffffffff);
	pos+=4;
	putle32(&section_data[pos], 0xffffffff);
	pos+=4;

	putle32(&section_data[pos], 0x10000);
	pos+=4;
	putle32(&section_data[pos], 0x0);
	pos+=4;
	putle32(&section_data[pos], 0x0);
	pos+=4;

	tmpfloat = 1.0f;
	putle32(&section_data[pos], *floatptr);
	pos+=4;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);//Don't know for sure if these two are floats or not.
	pos+=4;

	tmpfloat = 0.0f;
	putle32(&section_data[pos], *floatptr);
	pos+=4;

	tmpfloat = 1.0f;
	putle32(&section_data[pos], *floatptr);
	pos+=4;

	tmpfloat = 1.0f;
	putle32(&section_data[pos], *floatptr);
	pos+=4;

	tmpfloat = 1.0f;
	putle32(&section_data[pos], *floatptr);
	pos+=4;

	return layout_addsection(0x31636970, section_data, 0x78);
}

int layout_addgroupsection(char *groupname, u32 total_panerefs, char **panerefs)
{
	u32 pos;
	u32 size = 0x14;
	u8 section_data[0x34];

	if(strlen(groupname) > 0xf)
	{
		printf("The group name is too long.\n");
		return 6;
	}

	if(total_panerefs > 2)
	{
		printf("The total_panerefs is too large.\n");
		return 6;
	}

	memset(section_data, 0, sizeof(section_data));

	strncpy((char*)&section_data[0], groupname, 0xf);

	if(total_panerefs)
	{
		putle32(&section_data[0x10], total_panerefs);
		for(pos=0; pos<total_panerefs; pos++)
		{
			if(strlen(panerefs[pos]) > 0xf)
			{
				printf("panerefs[0x%x] is too long.\n", pos);
				return 6;
			}

			strncpy((char*)&section_data[size], panerefs[pos], 0xf);
			size+= 0x10;
		}
	}

	return layout_addsection(0x31707267, section_data, size);
}

int layout_startgroup()
{
	return layout_addsection(0x31737267, NULL, 0);
}

int layout_endgroup()
{
	return layout_addsection(0x31657267, NULL, 0);
}

int anim_buildsection_pat1(char *scene_name, char *group_name)
{
	u32 size;
	u32 pos;
	u8 section_data[0x34];

	memset(section_data, 0, sizeof(section_data));

	pos = 0;

	putle32(&section_data[pos], 0x10000);//Some sort of type field presumably?
	pos+=4;

	putle32(&section_data[pos], 0x1c);//Offset to the scene-name string, relative to the start of the pat1 magicnum.
	pos+=4;

	putle32(&section_data[pos], 0x28);//Offset to the group-name string, relative to the start of the pat1 magicnum.
	pos+=4;

	putle32(&section_data[pos], -15);//Seems to be related to the time value in the pai1 section?
	pos+=4;

	putle32(&section_data[pos], 0x1);//Unknown
	pos+=4;

	strncpy((char*)&section_data[pos], scene_name, 0xb);
	pos+= 0xc;

	strncpy((char*)&section_data[pos], group_name, 0x13);
	pos+= 0x14;

	return layout_addsection(0x31746170, section_data, 0x34);
}

int anim_buildsection_pai1(char *mat_name, char *rootpane_name)
{
	u32 size;
	u32 pos;
	u8 section_data[0x44];

	memset(section_data, 0, sizeof(section_data));

	pos = 0;

	putle32(&section_data[pos], 15);//Time related.
	pos+=4;

	putle16(&section_data[pos], 0x0);//Unknown
	pos+=2;

	putle16(&section_data[pos], 0x2);//num_entries
	pos+=2;

	putle32(&section_data[pos], 0x14);//offset_entries (from magicnum)
	pos+=4;

	//Each entry is a u32 offset, relative to the above magicnum.

	putle32(&section_data[pos], 0x1c);//Offset to the actual entry data for mat_name, relative to the start of the pat1 magicnum.
	pos+=4;

	putle32(&section_data[pos], 0x28+0x18);//Offset to the actual entry data for the root pane, relative to the start of the pat1 magicnum.
	pos+=4;

	//Start of the entry for mat_name.
	strncpy((char*)&section_data[pos], mat_name, 0x13);
	pos+= 0x14;

	putle32(&section_data[pos], 0x0);//Total animation chunks.
	pos+=4;

	//Start of the entry for rootpane.
	strncpy((char*)&section_data[pos], rootpane_name, 0x13);
	pos+= 0x14;

	putle32(&section_data[pos], 0x0);//Total animation chunks.
	pos+=4;

	return layout_addsection(0x31696170, section_data, 0x44);
}

int build_layoutsections(char *texture_filename, float screen_width)
{
	int ret;
	float tmpfloat;
	u32 *floatptr = (u32*)&tmpfloat;
	u32 size;
	char *panerefs[2];
	u8 section_data[0x40];

	//lyt1
	memset(section_data, 0, sizeof(section_data));

	putle32(&section_data[0x0], 0x1);//origin
	tmpfloat = screen_width;//Canvas width
	putle32(&section_data[0x4], *floatptr);
	tmpfloat = 240.f;//Canvas height
	putle32(&section_data[0x8], *floatptr);

	ret = layout_addsection(0x3174796c, (u8*)section_data, 0xc);
	if(ret)return ret;

	//txl1
	memset(section_data, 0, sizeof(section_data));

	putle32(&section_data[0x0], 0x1);//total textures
	putle32(&section_data[0x4], 0x4);//Relative offset to the first texture filename.

	size = strlen(texture_filename)+1;
	size = (size+0x3) & ~0x3;
	size+= 0x8;
	if(size > sizeof(section_data))
	{
		printf("The texture filename is too long.\n");
		return 6;
	}

	strncpy((char*)&section_data[0x8], texture_filename, sizeof(section_data)-9);

	ret = layout_addsection(0x316c7874, section_data, size);
	if(ret)return ret;

	ret = layout_addmatsection("HbMat");
	if(ret)return ret;

	ret = layout_addpansection("HbRootPane", 0x1, screen_width, 240.f);
	if(ret)return ret;

	ret = layout_addpansection("HbRoot0", 0x3, 40.f, 40.f);
	if(ret)return ret;

	ret = layout_addpicsection("HbMat", 128.0f, 64.0f, 0.0f, -120.0f, 0.0f);
	if(ret)return ret;

	ret = layout_finishpane();
	if(ret)return ret;

	ret = layout_finishpane();
	if(ret)return ret;

	ret = layout_addgroupsection("HbRootGrp", 0, NULL);
	if(ret)return ret;

	ret = layout_startgroup();
	if(ret)return ret;

	panerefs[0] = "HbRoot0";
	panerefs[1] = "HbMat";
	ret = layout_addgroupsection("G_A_00", 2, panerefs);
	if(ret)return ret;

	panerefs[0] = "HbMat";
	ret = layout_addgroupsection("G_B_00", 1, panerefs);
	if(ret)return ret;

	panerefs[0] = "HbRoot0";
	ret = layout_addgroupsection("G_C_00", 1, panerefs);
	if(ret)return ret;

	ret = layout_endgroup();
	if(ret)return ret;

	return 0;
}

int build_animsections(char *scene_name, char *group_name)
{
	int ret;

	ret = anim_buildsection_pat1(scene_name, group_name);
	if(ret)return ret;

	ret = anim_buildsection_pai1("HbMat", "HbRoot0");
	if(ret)return ret;

	return 0;
}

int build_layout(char *layout_filename, char *texture_filename, float screen_width)
{
	int ret;
	FILE *f;
	size_t transfersize;
	char layout_path[256];

	memset(layout_path, 0, sizeof(layout_path));
	snprintf(layout_path, sizeof(layout_path)-1, "%s%cblyt%c%s", darc_dirpath, PATH_SEPERATOR, PATH_SEPERATOR, layout_filename);

	f = fopen(layout_path, "wb");
	if(f==NULL)
	{
		printf("Failed to open the following filepath for writing: %s\n", layout_path);
		return 2;
	}

	memset(filebuf, 0, filebuf_size);

	layout_header = (bclyt_header*)filebuf;

	putle32(layout_header->magicnum, 0x54594c43);
	putle16(layout_header->bom, 0xfeff);
	putle16(layout_header->headerlen, 0x14);
	putle32(layout_header->revision, 0x2020000);

	layout_pos = sizeof(bclyt_header);

	ret = build_layoutsections(texture_filename, screen_width);
	if(ret)
	{
		fclose(f);
		return ret;
	}

	putle32(layout_header->filesize, layout_pos);

	transfersize = fwrite(filebuf, 1, layout_pos, f);
	fclose(f);
	if(transfersize != layout_pos)
	{
		printf("Failed to write the layout data to the file.\n");
		return 4;
	}

	return 0;
}

int build_anim(char *anim_filename, char *scene_name, char *group_name)
{
	int ret;
	FILE *f;
	size_t transfersize;
	char anim_path[256];

	memset(anim_path, 0, sizeof(anim_path));
	snprintf(anim_path, sizeof(anim_path)-1, "%s%canim%c%s", darc_dirpath, PATH_SEPERATOR, PATH_SEPERATOR, anim_filename);

	f = fopen(anim_path, "wb");
	if(f==NULL)
	{
		printf("Failed to open the following filepath for writing: %s\n", anim_path);
		return 2;
	}

	memset(filebuf, 0, filebuf_size);

	layout_header = (bclyt_header*)filebuf;//.bclan has the exact same header as .bclyt besides the magicnum value, so just reuse the same structure.

	putle32(layout_header->magicnum, 0x4e414c43);
	putle16(layout_header->bom, 0xfeff);
	putle16(layout_header->headerlen, 0x14);
	putle32(layout_header->revision, 0x2020000);

	layout_pos = sizeof(bclyt_header);

	ret = build_animsections(scene_name, group_name);
	if(ret)
	{
		fclose(f);
		return ret;
	}

	putle32(layout_header->filesize, layout_pos);

	transfersize = fwrite(filebuf, 1, layout_pos, f);
	fclose(f);
	if(transfersize != layout_pos)
	{
		printf("Failed to write the anim data to the file.\n");
		return 4;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int argi;
	int ret;
	u32 pos, pos2;

	char *bottomtex_filename = NULL, *toptex_filename = NULL;

	char tmpstr0[0x40];
	char tmpstr1[0x40];
	char tmpstr2[0x40];

	if(argc==1)
	{
		printf("%s by yellows8\n", argv[0]);
		printf("Tool for building the data used with 3DS ExeFS:/logo.\n");
		printf("Usage:\n");
		printf("--dirpath=<path> Required, this is the path where the darc directory stucture is located.\n");
		printf("--bottomtex=<path> Filename for the bottom-screen texture, under the timg directory.\n");
		printf("--toptex=<path> Filename for the top-screen texture, under the timg directory. These two image filenames are *only* used for writing into the .bclyt, this tool will not verify that the specified file actually exists.\n");
		return 0;
	}

	for(argi=1; argi<argc; argi++)
	{
		if(strncmp(argv[argi], "--dirpath=", 10)==0)darc_dirpath = &argv[argi][10];
		if(strncmp(argv[argi], "--bottomtex=", 12)==0)bottomtex_filename = &argv[argi][12];
		if(strncmp(argv[argi], "--toptex=", 9)==0)toptex_filename = &argv[argi][9];
	}

	if(darc_dirpath==NULL)
	{
		printf("Specify the dirpath.\n");
		return 1;
	}

	if(bottomtex_filename == NULL || toptex_filename == NULL)
	{
		printf("Specify --bottomtex and --toptex.\n");
		return 1;
	}

	filebuf = malloc(0x4000);
	if(filebuf==NULL)
	{
		printf("Failed to allocate memory for the filebuf.\n");
		free(filebuf);
		return 3;
	}

	ret = build_layout("NintendoLogo_D_00.bclyt", bottomtex_filename, 320.f);
	if(ret)
	{
		free(filebuf);
		return ret;
	}

	ret = build_layout("NintendoLogo_U_00.bclyt", toptex_filename, 400.0f);
	if(ret)
	{
		free(filebuf);
		return ret;
	}

	for(pos=0; pos<2; pos++)
	{
		for(pos2=0; pos2<3; pos2++)
		{
			memset(tmpstr0, 0, sizeof(tmpstr0));
			memset(tmpstr1, 0, sizeof(tmpstr1));
			memset(tmpstr2, 0, sizeof(tmpstr2));

			snprintf(tmpstr2, sizeof(tmpstr2)-1, "G_%c_00", 'A' + pos2);
			snprintf(tmpstr1, sizeof(tmpstr1)-1, "SceneOut%c", 'A' + pos2);
			snprintf(tmpstr0, sizeof(tmpstr0)-1, "NintendoLogo_%c_00_%s.bclan", pos==0?'D':'U', tmpstr1);

			ret = build_anim(tmpstr0, tmpstr1, tmpstr2);
			if(ret)break;
		}
	}

	free(filebuf);

	return ret;
}

