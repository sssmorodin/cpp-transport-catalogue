#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, const StrokeLineCap stroke_linecap) {
    switch (stroke_linecap) {
        case StrokeLineCap::BUTT:
            return output << "butt"s;
        case StrokeLineCap::ROUND:
            return output << "round"s;
        case StrokeLineCap::SQUARE:
            return output << "square"s;
    }
    return output;
}

std::ostream& operator<<(std::ostream& output, const StrokeLineJoin stroke_linejoin) {
    switch (stroke_linejoin) {
        case StrokeLineJoin::ARCS:
            return output << "arcs"s;
        case StrokeLineJoin::BEVEL:
            return output << "bevel"s;
        case StrokeLineJoin::MITER:
            return output << "miter"s;
        case StrokeLineJoin::MITER_CLIP:
            return output << "miter-clip"s;
        case StrokeLineJoin::ROUND:
            return output << "round"s;
    }
    return output;
}

std::ostream& operator<<(std::ostream& output, const Color color) {
    std::visit(ColorPrinter{output}, color);
    return output;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}


// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "  <circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "  <polyline points=\""sv;
    bool is_first = true;
    for (const auto& point : points_) {
        if (!is_first) {
            out << ' ';
        }
        is_first = false;
        out << point.x << ',' << point.y;
    }
    out << "\""sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}
Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}
void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "  <text";
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y;
    out << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y;
    out << "\" font-size=\""sv << font_size_;
    if (!font_family_.empty()) {
        out << "\" font-family=\""sv << font_family_;
    }
    if (!font_weight_.empty()) {
        out << "\" font-weight=\""sv << font_weight_;
    }
    out << "\">"sv;
    for (const char c : data_) {
        switch (c) {
            case '\"':
                out << "&quot;"sv;
                continue;
            case  '<':
                out << "&lt;"sv;
                continue;
            case  '>':
                out << "&gt;"sv;
                continue;
            case '\'':
                out << "&apos;"sv;
                continue;
            case  '&':
                out << "&amp;"sv;
                continue;
            default:
                out << c;
        }
    }
    out << "</text>";
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << std::endl;
    for (const auto& object : objects_) {
        object->Render(out);
    }
    out << "</svg>";
}

}  // namespace svg