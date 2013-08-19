#ifndef MINIFB_HPP
#define MINIFB_HPP

#include <string>
#include <map>
#include <linux/fb.h>

class MiniFB
{
   private:
      int fd;
      struct fb_var_screeninfo screeninfo;
      int width;
      int height;
      unsigned char* data;
      unsigned char* buffer;
      std::map<char, bool*> characterMap;

   public:
      MiniFB(std::string fb);
      ~MiniFB();
      void clearScreen(void);
      void updateScreen(void);
      void drawPixel(int x, int y, unsigned char color);
      void drawLine(int x1, int y1, int x2, int y2, unsigned char color);
      void drawRectangleFill(int x1, int y1, int x2, int y2, unsigned char color);
      void drawRectangleBorder(int x1, int y1, int x2, int y2, unsigned char color);
      void drawRectangle(int x1, int y1, int x2, int y2, unsigned char fillColor, unsigned char borderColor);
      void drawText(int x, int y, std::string text, unsigned char color);
      int getTextWidth(std::string text);
      int getTextHeight(std::string text);
      unsigned char thinColor(unsigned char color, int times);

   private:
      void addCharacter(char character);
};

#endif

