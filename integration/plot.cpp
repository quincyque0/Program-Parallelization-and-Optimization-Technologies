#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <algorithm>

struct DataPoint {
    int p;
    double time;
    double speedup;
    double efficiency;
};

std::vector<DataPoint> load_csv(const std::string& path) {
    std::vector<DataPoint> data;
    std::ifstream in(path);
    if (!in.is_open()) return data;

    std::string line;
    std::getline(in, line); 

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string val;
        DataPoint pt;

        std::getline(ss, val, ','); pt.p = std::stoi(val);
        std::getline(ss, val, ','); pt.time = std::stod(val);
        std::getline(ss, val, ','); pt.speedup = std::stod(val);
        std::getline(ss, val, ','); pt.efficiency = std::stod(val);

        data.push_back(pt);
    }
    return data;
}

void build_svg(const std::string& filename, const std::vector<DataPoint>& pts) {
    int total_w = 1100, total_h = 480;
    int chart_w = 400, chart_h = 300;
    int pad_top = 70, pad_bottom = 90;
    int x1 = 80, x2 = 600;

    std::ofstream out(filename);
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<svg width=\"" << total_w << "\" height=\"" << total_h << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
    out << "<style>\n"
        << "  .title { font: bold 15px sans-serif; fill: #111; text-anchor: middle; }\n"
        << "  .axis-title { font: 12px sans-serif; fill: #333; text-anchor: middle; }\n"
        << "  .tick { font: 11px monospace; fill: #444; }\n"
        << "  .grid { stroke: #e5e7eb; stroke-width: 1; }\n"
        << "  .legend { font: 12px sans-serif; fill: #333; }\n"
        << "</style>\n";

    out << "<rect width=\"100%\" height=\"100%\" fill=\"#ffffff\"/>\n";

    int n = static_cast<int>(pts.size());
    auto get_px = [&](int idx, int left_offset) -> double {
        return left_offset + (static_cast<double>(idx) / (n - 1)) * chart_w;
    };
    double max_s = 0.0;
    for (const auto& d : pts) max_s = std::max({max_s, d.speedup, static_cast<double>(d.p)});
    double y_max_s = max_s * 1.1;

    out << "<text x=\"" << (x1 + chart_w / 2) << "\" y=\"35\" class=\"title\">Ускорение S(p)</text>\n";

    for (int i = 0; i <= 5; ++i) {
        double val = (y_max_s / 5.0) * i;
        double y = (pad_top + chart_h) - (val / y_max_s) * chart_h;
        out << "<line x1=\"" << x1 << "\" y1=\"" << y << "\" x2=\"" << (x1 + chart_w) << "\" y2=\"" << y << "\" class=\"grid\"/>\n";
        out << "<text x=\"" << (x1 - 10) << "\" y=\"" << (y + 4) << "\" class=\"tick\" text-anchor=\"end\">"
            << std::fixed << std::setprecision(1) << val << "</text>\n";
    }
    out << "<polyline fill=\"none\" stroke=\"#9ca3af\" stroke-width=\"2\" stroke-dasharray=\"5,5\" points=\"";
    for (int i = 0; i < n; ++i) {
        double y = (pad_top + chart_h) - (static_cast<double>(pts[i].p) / y_max_s) * chart_h;
        out << get_px(i, x1) << "," << y << " ";
    }
    out << "\"/>\n";

    out << "<polyline fill=\"none\" stroke=\"#2563eb\" stroke-width=\"3\" points=\"";
    for (int i = 0; i < n; ++i) {
        double y = (pad_top + chart_h) - (pts[i].speedup / y_max_s) * chart_h;
        out << get_px(i, x1) << "," << y << " ";
    }
    out << "\"/>\n";

    for (int i = 0; i < n; ++i) {
        double px = get_px(i, x1);
        double py = (pad_top + chart_h) - (pts[i].speedup / y_max_s) * chart_h;
        out << "<circle cx=\"" << px << "\" cy=\"" << py << "\" r=\"4.5\" fill=\"#2563eb\"/>\n";
        out << "<text x=\"" << px << "\" y=\"" << (py - 10) << "\" class=\"tick\" font-weight=\"bold\" fill=\"#1e40af\" text-anchor=\"middle\">"
            << std::fixed << std::setprecision(2) << pts[i].speedup << "x</text>\n";
        out << "<text x=\"" << px << "\" y=\"" << (pad_top + chart_h + 20) << "\" class=\"tick\" text-anchor=\"middle\">" << pts[i].p << "</text>\n";
    }

    out << "<line x1=\"" << x1 << "\" y1=\"" << pad_top << "\" x2=\"" << x1 << "\" y2=\"" << (pad_top + chart_h) << "\" stroke=\"#111\" stroke-width=\"1.5\"/>\n";
    out << "<line x1=\"" << x1 << "\" y1=\"" << (pad_top + chart_h) << "\" x2=\"" << (x1 + chart_w) << "\" y2=\"" << (pad_top + chart_h) << "\" stroke=\"#111\" stroke-width=\"1.5\"/>\n";
    out << "<text x=\"" << (x1 + chart_w / 2) << "\" y=\"" << (pad_top + chart_h + 45) << "\" class=\"axis-title\">Число потоков (p)</text>\n";

    out << "<line x1=\"" << x1 << "\" y1=\"" << (pad_top + chart_h + 70) << "\" x2=\"" << (x1 + 25) << "\" y2=\"" << (pad_top + chart_h + 70) << "\" stroke=\"#2563eb\" stroke-width=\"3\"/>\n";
    out << "<text x=\"" << (x1 + 35) << "\" y=\"" << (pad_top + chart_h + 74) << "\" class=\"legend\">Реальное S(p)</text>\n";
    out << "<line x1=\"" << (x1 + 180) << "\" y1=\"" << (pad_top + chart_h + 70) << "\" x2=\"" << (x1 + 205) << "\" y2=\"" << (pad_top + chart_h + 70) << "\" stroke=\"#9ca3af\" stroke-width=\"2\" stroke-dasharray=\"4,4\"/>\n";
    out << "<text x=\"" << (x1 + 215) << "\" y=\"" << (pad_top + chart_h + 74) << "\" class=\"legend\">Идеальное (S = p)</text>\n";

    double max_t = 0.0;
    for (const auto& d : pts) max_t = std::max(max_t, d.time);
    double y_max_t = max_t * 1.15;

    out << "<text x=\"" << (x2 + chart_w / 2) << "\" y=\"35\" class=\"title\">Время работы T(p)</text>\n";

    for (int i = 0; i <= 5; ++i) {
        double val = (y_max_t / 5.0) * i;
        double y = (pad_top + chart_h) - (val / y_max_t) * chart_h;
        out << "<line x1=\"" << x2 << "\" y1=\"" << y << "\" x2=\"" << (x2 + chart_w) << "\" y2=\"" << y << "\" class=\"grid\"/>\n";
        out << "<text x=\"" << (x2 - 10) << "\" y=\"" << (y + 4) << "\" class=\"tick\" text-anchor=\"end\">"
            << std::fixed << std::setprecision(3) << val << " с</text>\n";
    }

    out << "<polyline fill=\"none\" stroke=\"#dc2626\" stroke-width=\"3\" points=\"";
    for (int i = 0; i < n; ++i) {
        double y = (pad_top + chart_h) - (pts[i].time / y_max_t) * chart_h;
        out << get_px(i, x2) << "," << y << " ";
    }
    out << "\"/>\n";

    for (int i = 0; i < n; ++i) {
        double px = get_px(i, x2);
        double py = (pad_top + chart_h) - (pts[i].time / y_max_t) * chart_h;
        out << "<circle cx=\"" << px << "\" cy=\"" << py << "\" r=\"4.5\" fill=\"#dc2626\"/>\n";
        out << "<text x=\"" << px << "\" y=\"" << (py - 10) << "\" class=\"tick\" font-weight=\"bold\" fill=\"#991b1b\" text-anchor=\"middle\">"
            << std::fixed << std::setprecision(4) << pts[i].time << "</text>\n";
        out << "<text x=\"" << px << "\" y=\"" << (pad_top + chart_h + 20) << "\" class=\"tick\" text-anchor=\"middle\">" << pts[i].p << "</text>\n";
    }

    out << "<line x1=\"" << x2 << "\" y1=\"" << pad_top << "\" x2=\"" << x2 << "\" y2=\"" << (pad_top + chart_h) << "\" stroke=\"#111\" stroke-width=\"1.5\"/>\n";
    out << "<line x1=\"" << x2 << "\" y1=\"" << (pad_top + chart_h) << "\" x2=\"" << (x2 + chart_w) << "\" y2=\"" << (pad_top + chart_h) << "\" stroke=\"#111\" stroke-width=\"1.5\"/>\n";
    out << "<text x=\"" << (x2 + chart_w / 2) << "\" y=\"" << (pad_top + chart_h + 45) << "\" class=\"axis-title\">Число потоков (p)</text>\n";

    out << "<line x1=\"" << x2 << "\" y1=\"" << (pad_top + chart_h + 70) << "\" x2=\"" << (x2 + 25) << "\" y2=\"" << (pad_top + chart_h + 70) << "\" stroke=\"#dc2626\" stroke-width=\"3\"/>\n";
    out << "<text x=\"" << (x2 + 35) << "\" y=\"" << (pad_top + chart_h + 74) << "\" class=\"legend\">Время T(p) в сек.</text>\n";

    out << "</svg>\n";
}

int main(int argc, char* argv[]) {
    std::string csv_in = (argc > 1) ? argv[1] : "results.csv";
    std::string svg_out = (argc > 2) ? argv[2] : "scalability.svg";

    auto data = load_csv(csv_in);
    if (data.empty()) {
        std::cerr << "Файл " << csv_in << " пуст или не найден.\n";
        return 1;
    }

    build_svg(svg_out, data);
    std::cout << "График успешно сгенерирован: " << svg_out << "\n";
    return 0;
}