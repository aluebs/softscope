#include <MiniFB.hpp>
#include <fix.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <cstdlib>
#include <sstream>
#include <fstream>

#define FONT_SIZE 8

using namespace std;

MiniFB::MiniFB(string fb)
{
   fd = open(fb.data(), O_RDWR);
   ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo);
   width = screeninfo.xres;
   height = screeninfo.yres;
   data = (unsigned char*) mmap(0, 2 * width * height, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   buffer = data;
   for(char c = 'a'; c <= 'z'; c++)
   {
      addCharacter(c);
   }
   for(char c = 'A'; c <= 'Z'; c++)
   {
      addCharacter(c);
   }
   for(char c = '0'; c <= '9'; c++)
   {
      addCharacter(c);
   }
   addCharacter(' ');
   addCharacter('-');
   addCharacter('+');
   addCharacter('.');
   addCharacter('/');
   addCharacter(':');
   addCharacter(';');
   addCharacter('=');
   addCharacter('%');
}

MiniFB::~MiniFB()
{
   munmap(data, width * height);
   close(fd);
   for(map<char, bool*>::iterator i = characterMap.begin(); i != characterMap.end(); i++)
   {
      delete[] i->second;
   }
}

void MiniFB::clearScreen(void)
{
   drawRectangleFill(0, 0, width - 1, height - 1, 0x00);
}

void MiniFB::updateScreen(void)
{
   if(buffer == data)
   {
      buffer += width * height;
      screeninfo.yoffset = 0;
   }
   else
   {
      buffer = data;
      screeninfo.yoffset = height;
   }
   ioctl(fd, FBIOPAN_DISPLAY, &screeninfo);
}

void MiniFB::drawPixel(int x, int y, unsigned char color)
{
   if((x >= 0) && (x < width) && (y >= 0) && (y < height))
   {
      buffer[y * width + x] = color;
   }
}

void MiniFB::drawLine(int x1, int y1, int x2, int y2, unsigned char color)
{
   if((abs(x2 - x1) == 0) && (abs(y2 - y1) == 0))
   {
      drawPixel(x1, y1, color);
   }
   else if(abs(x2 - x1) > abs(y2 - y1))
   {
      int xStep = (x1 < x2) ? 1 : -1;
      fix y = y1 + 0.5f;
      fix yStep = fix(y2 - y1) / (x2 - x1);
      for(int x = x1; x != x2; x += xStep)
      {
         drawPixel(x, y, color);
         y += yStep;
      }
   }
   else
   {
      int yStep = (y1 < y2) ? 1 : -1;
      fix x = x1 + 0.5f;
      fix xStep = fix(x2 - x1) / (y2 - y1);
      for(int y = y1; y != y2; y += yStep)
      {
         drawPixel(x, y, color);
         x += xStep;
      }
   }
}

void MiniFB::drawRectangleFill(int x1, int y1, int x2, int y2, unsigned char color)
{
   int xStep = (x1 > x2) ? -1 : 1;
   int yStep = (y1 > y2) ? -1 : 1;
   for(int x = x1; x <= x2; x += xStep)
   {
      for(int y = y1; y <= y2; y += yStep)
      {
         drawPixel(x, y, color);
      }
   }
}

void MiniFB::drawRectangleBorder(int x1, int y1, int x2, int y2, unsigned char color)
{
   drawLine(x1, y1, x2, y1, color);
   drawLine(x1, y2, x2, y2, color);
   drawLine(x1, y1, x1, y2, color);
   drawLine(x2, y1, x2, y2, color);
}

void MiniFB::drawRectangle(int x1, int y1, int x2, int y2, unsigned char fillColor, unsigned char borderColor)
{
   drawRectangleFill(x1, y1, x2, y2, fillColor);
   drawRectangleBorder(x1, y1, x2, y2, borderColor);
}

void MiniFB::drawText(int x, int y, string text, unsigned char color)
{
   int xt = x;
   int yt = y;
   for(int i =0; i < text.size(); i++)
   {
      char character = text[i];
      if(character == '\n')
      {
         yt += 5 * FONT_SIZE / 4;
         xt = x;
      }
      else
      {
         bool* pixels = characterMap[character];
         for(int j = 0; j < FONT_SIZE; j++)
         {
            for(int k = 0; k < FONT_SIZE; k++)
            {
               if(pixels[j + k * FONT_SIZE])
               {
                  drawPixel(xt + j, yt + k, color);
               }
            }
         }
         xt += FONT_SIZE;
      }
   }
}

int MiniFB::getTextWidth(string text)
{
   int width = 0;
   int tmpWidth = 0;
   for(int i = 0; i < text.size(); i++)
   {
      if(text[i] == '\n')
      {
         if(tmpWidth > width)
         {
            width = tmpWidth;
         }
         tmpWidth = 0;
      }
      else
      {
         tmpWidth += FONT_SIZE;
      }
   }
   if(tmpWidth > width)
   {
      width = tmpWidth;
   }
   return width;
}

int MiniFB::getTextHeight(string text)
{
   int height = FONT_SIZE;
   for(int i =0; i < text.size(); i++)
   {
      if(text[i] == '\n')
      {
         height += 5 * FONT_SIZE / 4;
      }
   }
   return height;
}

unsigned char MiniFB::thinColor(unsigned char color, int times)
{
   unsigned char r = color & 0xE0;
   unsigned char g = color & 0x18;
   unsigned char b = color & 0x07;
   r >>= times;
   g >>= times;
   b >>= times;
   r &= 0xE0;
   g &= 0x18;
   b &= 0x07;
   return r | g | b;
}

void MiniFB::addCharacter(char character)
{
   bool* pixels = new bool[FONT_SIZE * FONT_SIZE];
   stringstream ss;
   ss << "font/";
   switch(character)
   {
      case '/':
         ss << "slash";
         break;
      default:
         ss << character;
         break;
   }
   ss << ".txt";
   ifstream myFile(ss.str().data());
   string line;
   if(myFile.is_open())
   {
      for(int i = 0; i < FONT_SIZE; i++)
      {
         getline(myFile, line);
         for(int j = 0; j < FONT_SIZE; j++)
         {
            pixels[i * FONT_SIZE + j] = (line[j] == 'X');
         }
      }
      myFile.close();
   }
   characterMap[character] = pixels;
}

