/*

BERGCloud library for mbed

Copyright (c) 2013 BERG Cloud Ltd. http://bergcloud.com/

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include <cstdint>
#include <cstddef>
#include <cstdarg>

#include "BERGCloudMbed.h"

uint16_t BERGCloudMbed::SPITransaction(uint8_t *dataOut, uint8_t *dataIn, uint16_t dataSize, bool finalCS)
{
  uint16_t i;

  if ( (dataOut == NULL) || (dataIn == NULL) || (spi == NULL) )
  {
    _LOG("Invalid parameter (CBERGCloud::SPITransaction)\r\n");
    return 0;
  }

  nSSELPin->write(0);

  for (i = 0; i < dataSize; i++)
  {
    *dataIn++ = spi->write(*dataOut++);
  }

  if (finalCS)
  {
    nSSELPin->write(1);
  }

  return dataSize;
}

void BERGCloudMbed::timerReset(void)
{
  timer->reset();
}

uint32_t BERGCloudMbed::timerRead_mS(void)
{
  return timer->read_ms();
}

void BERGCloudMbed::begin(PinName _MOSIPin, PinName _MISOPin, PinName _SCLKPin, PinName _nSSELPin)
{
  /* Call base class method */
  BERGCloudBase::begin();

  /* Configure nSSEL control pin */
  nSSELPin = new DigitalOut(_nSSELPin);

  if (nSSELPin == NULL)
  {
    _LOG("nSSELPin is NULL (CBERGCloud::begin)\r\n");
    return;
  }

  nSSELPin->write(1);

  /* Configure SPI */
  spi = new SPI(_MOSIPin, _MISOPin, _SCLKPin);

  if (spi  == NULL)
  {
    _LOG("spi is NULL (CBERGCloud::begin)\r\n");
    delete nSSELPin;
    return;
  }

  spi->format(8, 0); /* 8-bits; SPI MODE 0 */
  spi->frequency(4000000); /* 4MHz */

  /* Configure timer */
  timer = new Timer();

  if (timer  == NULL)
  {
    _LOG("timer is NULL (CBERGCloud::begin)\r\n");
    delete nSSELPin;
    delete spi;
    return;
  }

  timer->start();
}

void BERGCloudMbed::end()
{
  if (nSSELPin != NULL)
  {
    delete nSSELPin;
  }

  if (spi != NULL)
  {
    delete spi;
  }

  if (timer != NULL)
  {
    delete timer;
  }

  /* Call base class method */
  BERGCloudBase::end();
}

bool BERGCloudMbed::display(std::string& s)
{
  return display(s.c_str());
}

uint16_t BERGCloudMbed::getHostType(void)
{
  return BC_HOST_MBED;
}

#ifdef BERGCLOUD_PACK_UNPACK

bool BERGCloudMessage::pack(std::string& s)
{
  return pack(s.c_str());
}

bool BERGCloudMessage::unpack(std::string& s)
{
  uint16_t sizeInBytes;

  if (!unpack_raw_header(&sizeInBytes))
  {
    return false;
  }

  if (!remaining(sizeInBytes))
  {
    _LOG_UNPACK_ERROR_NO_DATA;
    return false;
  }

  s.clear();
  s.append((const char *)&buffer[bytesRead], sizeInBytes);
  bytesRead += sizeInBytes;

  return true;
}

bool BERGCloudMbed::pollForCommand(BERGCloudMessageBuffer& buffer, string &commandName)
{
  bool result = false;
  char tmp[31 + 1]; /* +1 for null terminator */

  commandName = ""; /* Empty string */
  result = pollForCommand(buffer, tmp, sizeof(tmp));

  if (result)
  {
    commandName = string(tmp);
  }

  return result;
}

#endif // #ifdef BERGCLOUD_PACK_UNPACK
