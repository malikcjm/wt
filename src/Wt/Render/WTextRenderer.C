/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WPainter>
#include <Wt/Render/WTextRenderer>

#include "Block.h"

namespace Wt {
  namespace Render {

WTextRenderer::WTextRenderer()
  : device_(0),
    fontScale_(1)
{ }

WTextRenderer::~WTextRenderer()
{ }

void WTextRenderer::setFontScale(double factor)
{
  fontScale_ = factor;
}

double WTextRenderer::textWidth(int page) const
{
  return pageWidth(page) - margin(Left) - margin(Right);
}

double WTextRenderer::textHeight(int page) const
{
  return pageHeight(page) - margin(Top) - margin(Bottom);
}

double WTextRenderer::render(const WString& text, double y)
{
  std::string xhtml = text.toUTF8();
  char *cxhtml = const_cast<char *>(xhtml.c_str()); // Shhht it's okay !

  try {
    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_xhtml_entity_translation>(cxhtml);

    Block docBlock(&doc, 0);

    docBlock.determineDisplay();
    docBlock.normalizeWhitespace(false, doc);

    double currentY = y;
    int currentPage = 0;

    BlockList floats;

    double minX = 0;
    double maxX = textWidth(currentPage);

    device_ = startPage(currentPage);
    painter_ = new WPainter(device_);

    WFont defaultFont;
    defaultFont.setFamily(WFont::SansSerif);
    painter_->setFont(defaultFont);

    double collapseMarginBottom;
    docBlock.layoutBlock(currentY, currentPage, floats, minX, maxX,
			 false, *this, std::numeric_limits<double>::max(),
			 collapseMarginBottom);
    Block::clearFloats(currentY, currentPage, floats, minX, maxX, maxX - minX);

    for (int page = 0; page <= currentPage; ++page) {
      if (page != 0) {
	device_ = startPage(page);
	painter_ = new WPainter(device_);
	painter_->setFont(defaultFont);
      }

      docBlock.render(*this, page);

      delete painter_;
      endPage(device_);
    }

    return currentY;
  } catch (rapidxml::parse_error& e) {
    throw e;
  }
}

  }
}