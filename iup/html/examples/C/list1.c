/* IupList: Example in C 
   Creates a dialog with three frames, each one containing a list.
   The first is a simple list, the second one is a multiple list and the last one is a drop-down list.
   The second list has a callback associated.
*/

#include <stdlib.h>
#include <stdio.h>
#include <iup.h>

static Ihandle* load_image_LogoTecgraf(void)
{
  unsigned char imgdata[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 108, 120, 143, 125, 132, 148, 178, 173, 133, 149, 178, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100, 110, 130, 48, 130, 147, 177, 254, 124, 139, 167, 254, 131, 147, 176, 137, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 115, 128, 153, 134, 142, 159, 191, 194, 47, 52, 61, 110, 114, 128, 154, 222, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 143, 172, 192, 140, 156, 188, 99, 65, 69, 76, 16, 97, 109, 131, 251, 129, 144, 172, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 131, 147, 175, 232, 140, 157, 188, 43, 0, 0, 0, 0, 100, 112, 134, 211, 126, 141, 169, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 72, 78, 88, 26, 48, 52, 57, 60, 135, 150, 178, 254, 108, 121, 145, 83, 105, 118, 142, 76, 106, 119, 143, 201, 118, 133, 159, 122, 117, 129, 152, 25, 168, 176, 190, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    118, 128, 145, 3, 104, 117, 140, 92, 114, 127, 152, 180, 131, 147, 177, 237, 133, 149, 178, 249, 38, 42, 50, 222, 137, 152, 180, 249, 126, 142, 170, 182, 114, 128, 154, 182, 104, 117, 140, 227, 95, 107, 128, 238, 83, 93, 112, 248, 84, 95, 113, 239, 104, 117, 141, 180, 115, 129, 155, 93, 127, 140, 165, 4,
    98, 109, 130, 153, 109, 123, 147, 254, 145, 163, 195, 153, 138, 154, 182, 56, 115, 123, 138, 5, 92, 99, 109, 35, 134, 149, 177, 230, 0, 0, 0, 0, 0, 0, 0, 0, 120, 133, 159, 143, 135, 151, 181, 115, 86, 89, 93, 5, 41, 45, 51, 54, 40, 45, 53, 150, 107, 120, 144, 254, 122, 137, 164, 154,
    51, 57, 66, 147, 83, 93, 112, 255, 108, 121, 145, 159, 113, 126, 151, 62, 123, 136, 159, 8, 87, 93, 103, 35, 125, 141, 169, 230, 0, 0, 0, 0, 0, 0, 0, 0, 129, 143, 169, 143, 140, 156, 184, 115, 134, 147, 172, 8, 124, 138, 165, 60, 124, 139, 167, 155, 131, 147, 177, 255, 131, 147, 176, 153,
    64, 68, 73, 2, 36, 39, 45, 86, 41, 46, 54, 173, 60, 67, 80, 232, 75, 84, 101, 251, 89, 100, 120, 228, 105, 118, 142, 250, 110, 123, 148, 187, 118, 132, 158, 187, 126, 141, 169, 229, 134, 149, 177, 239, 136, 152, 179, 250, 136, 152, 181, 234, 139, 156, 186, 175, 130, 145, 173, 90, 124, 134, 151, 3,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 71, 74, 79, 19, 60, 64, 73, 50, 92, 103, 124, 254, 86, 95, 111, 84, 90, 100, 117, 76, 126, 141, 168, 201, 113, 126, 150, 119, 99, 105, 117, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 93, 105, 125, 231, 135, 151, 181, 46, 0, 0, 0, 0, 137, 154, 184, 212, 123, 137, 164, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 74, 83, 98, 191, 133, 149, 179, 102, 111, 121, 139, 17, 134, 150, 180, 252, 126, 140, 166, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 43, 48, 57, 132, 121, 136, 164, 197, 121, 135, 161, 115, 130, 146, 175, 221, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 43, 47, 52, 46, 87, 98, 118, 254, 126, 142, 170, 254, 124, 139, 166, 135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 51, 57, 67, 118, 115, 128, 152, 170, 127, 140, 164, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    Ihandle* image = IupImageRGBA(16, 16, imgdata);
    return image;
}

static void load_medal_images(void)
{
  Ihandle *image_gold, *image_silver, *image_bronze;  

  unsigned char img_gold[16*16] =
  {
    0,0,0,4,4,4,4,4,4,4,4,4,4,0,0,0,
    0,0,4,4,4,4,4,4,4,4,4,4,4,4,0,0,
    0,0,4,4,4,4,4,4,4,4,4,4,4,4,0,0,
    0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,
    0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,
    0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,
    0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,
    0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,
    0,3,4,4,2,4,4,4,4,4,2,2,4,4,3,0,
    2,0,2,2,2,2,2,2,2,2,2,2,2,2,0,2,
    0,0,3,3,2,2,2,1,1,2,2,2,1,3,3,0,
    0,1,1,1,3,2,1,1,1,1,2,3,3,3,3,0,
    3,3,1,1,1,3,3,3,1,3,3,1,1,1,1,1,
    3,3,1,1,1,1,1,3,3,1,1,1,1,1,1,1,
    0,0,0,0,3,1,1,0,0,1,1,1,0,0,0,0,
    0,0,0,0,0,3,3,0,0,1,1,3,0,0,0,0
  };

  unsigned char img_silver[16*16] =
  {
    0,0,0,3,3,3,3,3,3,3,3,3,3,0,0,0,
    0,0,4,3,3,3,3,3,3,3,3,3,3,4,0,0,
    0,0,3,3,3,3,3,3,3,3,3,3,3,3,0,0,
    0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,
    0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,
    0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,
    0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,
    0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,
    0,3,3,3,3,3,3,3,3,3,3,2,3,3,3,0,
    3,0,2,2,2,2,2,3,3,2,2,2,2,2,0,3,
    0,0,1,1,2,2,1,1,1,2,2,2,1,1,3,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,
    0,0,0,0,2,1,1,0,0,0,1,1,0,0,0,0,
    0,0,0,0,0,1,1,0,0,1,1,2,0,0,0,0
  };

  unsigned char img_bronze[16*16] =
  {
    0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
    0,0,4,1,1,1,1,1,1,1,1,1,1,4,0,0,
    0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,3,3,1,1,1,1,3,3,1,1,1,0,
    4,0,3,3,3,3,3,3,3,3,3,3,3,2,0,4,
    0,0,3,3,3,3,2,2,2,2,3,2,2,3,4,0,
    0,2,2,2,2,3,2,2,2,2,2,2,2,2,3,0,
    4,3,2,2,2,2,2,2,2,2,2,2,2,2,2,3,
    4,3,2,2,2,2,2,2,4,2,2,2,2,2,2,3,
    0,0,0,0,3,2,2,0,0,2,2,2,0,0,0,0,
    0,0,0,0,0,3,3,0,0,2,2,4,0,0,0,0
  };

  image_gold = IupImage(16, 16, img_gold);
  IupSetAttribute(image_gold, "0", "BGCOLOR");
  IupSetAttribute(image_gold, "1", "128 0 0");
  IupSetAttribute(image_gold, "2", "128 128 0");
  IupSetAttribute(image_gold, "3", "255 0 0");
  IupSetAttribute(image_gold, "4", "255 255 0");
  IupSetHandle("IMGGOLD", image_gold);

  image_silver = IupImage(16, 16, img_silver);
  IupSetAttribute(image_silver, "0", "BGCOLOR");
  IupSetAttribute(image_silver, "1", "0 128 128");
  IupSetAttribute(image_silver, "2", "128 128 128");
  IupSetAttribute(image_silver, "3", "192 192 192");
  IupSetAttribute(image_silver, "4", "255 255 255");
  IupSetHandle("IMGSILVER", image_silver);

  image_bronze = IupImage(16, 16, img_bronze);
  IupSetAttribute(image_bronze, "0", "BGCOLOR");
  IupSetAttribute(image_bronze, "1", "128 0 0");
  IupSetAttribute(image_bronze, "2", "0 128 0");
  IupSetAttribute(image_bronze, "3", "128 128 0");
  IupSetAttribute(image_bronze, "4", "128 128 128");
  IupSetHandle("IMGBRONZE", image_bronze);
}

int list_multiple_cb (Ihandle *self, char *t, int i, int v)
{
  printf ("Item %d - %s - %s\n", i, t, v == 0 ? "deselected" : "selected" );
  return IUP_DEFAULT;
}

int main(int argc, char **argv) 
{
  Ihandle *dlg;
  Ihandle *list, *list_multiple, *list_dropdown;
  Ihandle *frm_medal, *frm_sport, *frm_prize;

  IupOpen(&argc, &argv);

  list = IupList ("list_act");
  IupSetAttributes (list, "1=Gold, 2=Silver, 3=Bronze, 4=None,"
                          "SHOWIMAGE=YES, VALUE=4, SIZE=EIGHTHxEIGHTH");
  load_medal_images();
  IupSetAttribute(list, "IMAGE0", "IMGGOLD");
  IupSetAttribute(list, "IMAGE1", "IMGSILVER");
  IupSetAttribute(list, "IMAGE2", "IMGBRONZE");

  frm_medal = IupFrame (list);
  IupSetAttribute (frm_medal, "TITLE", "Best medal");
  list_multiple = IupList(NULL);
  
  IupSetAttributes (list_multiple, "1=\"100m dash\", 2=\"Long jump\", 3=\"Javelin throw\", 4=\"110m hurdlers\", 5=\"Hammer throw\",6=\"High jump\","
                                   "MULTIPLE=YES, VALUE=\"+--+--\", SIZE=EIGHTHxEIGHTH");

  IupSetCallback(list_multiple, "ACTION", (Icallback)list_multiple_cb);
  
  frm_sport = IupFrame (list_multiple);
  
  IupSetAttribute (frm_sport, "TITLE", "Competed in");

  list_dropdown = IupList (NULL);
  IupSetAttributes (list_dropdown, "1=\"Less than US$ 1000\", 2=\"US$ 2000\", 3=\"US$ 5000\", 4=\"US$ 10000\", 5=\"US$ 20000\", 6=\"US$ 50000\", 7=\"More than US$ 100000\","
                                   "SHOWIMAGE=YES, DROPDOWN=YES, VISIBLE_ITEMS=5");
  IupSetAttributeHandle(list_dropdown, "DEFAULTIMAGE", load_image_LogoTecgraf());
  
  frm_prize = IupFrame (list_dropdown);
  IupSetAttribute (frm_prize, "TITLE", "Prizes won");

  dlg = IupDialog (IupHbox (frm_medal, frm_sport, frm_prize, NULL));
  IupSetAttribute (dlg, "TITLE", "IupList Example");
  IupShowXY (dlg, IUP_CENTER, IUP_CENTER);

  IupMainLoop ();
  IupClose ();
  return EXIT_SUCCESS;

}