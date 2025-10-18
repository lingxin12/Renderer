#pragma once
#include <iostream>
#include <chrono>
#include <thread>
#include <format>
#include <variant>

namespace lxrt {

class ProcessBar {
public:
    ProcessBar(): total_num_(100), now_num_(0), desc_(""), start_time_(std::chrono::high_resolution_clock::now()), last_time_(start_time_) {}
    ProcessBar(size_t total_num, std::string desc=""): 
        total_num_(total_num), now_num_(0), desc_(desc),
        start_time_(std::chrono::high_resolution_clock::now()), last_time_(start_time_) {
            process_bar_constructor();
        }
    
    void update(size_t now_num) {
        now_num_ = now_num;
        process_bar_constructor();
    }

    ~ProcessBar() {
        std::clog << std::endl;
    }

private:
    void process_bar_constructor() {
        std::clog << '\r' << desc_ << ":  " << static_cast<size_t>((float)now_num_/total_num_*100) << "%|" << get_process_bar_content()
                      << "| " << now_num_ << "/" << total_num_ << " ["
                      << std::get<std::string>(get_elapsed_time<std::string>()) << "<" << get_remain_time()
                      << ",  " << (now_num_ == 0 ? std::to_string(0) : std::format("{:.2f}", std::get<float>(get_elapsed_time<float>()) / now_num_)) << "s/it]"
                      << std::flush;
        last_time_ = std::chrono::high_resolution_clock::now();
    }

    std::string get_process_bar_content() const {
        if(now_num_ == 0) return std::string(process_bar_width_, '-');
        else if(now_num_ == total_num_) return std::string(process_bar_width_, '=');
        auto now_process_bar_width = static_cast<size_t>(static_cast<float>(now_num_)/total_num_*process_bar_width_);
        return std::string(now_process_bar_width, '=') + ">" + std::string(process_bar_width_-now_process_bar_width-1, '-');
    }

    template <typename T=float>
    auto get_elapsed_time() const -> std::variant<std::string, float> {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<float>(now - start_time_).count();
        if constexpr (std::is_same_v<T, float>) return duration;

        auto duration_size_t = static_cast<size_t>(duration);
        if(duration >= 3600) return std::format("{}:{:0>2}:{:0>2}", duration_size_t/3600, duration_size_t%3600/60, duration_size_t%60);
        else return std::format("{:0>2}:{:0>2}", duration_size_t/60, duration_size_t%60);
    }

    std::string get_remain_time() const {
        auto speed_time = std::get<float>(get_elapsed_time<float>()) / now_num_;
        auto remain_time = static_cast<size_t>(speed_time * (total_num_ - now_num_));
        if(remain_time >= 3600) return std::format("{:0>2}:{:0>2}:{:0>2}", remain_time/3600, remain_time%3600/60, remain_time%60);
        else return std::format("{:0>2}:{:0>2}", remain_time/60, remain_time%60);
    }

    size_t total_num_;
    size_t now_num_;
    std::string desc_;
    decltype(std::chrono::high_resolution_clock::now()) start_time_;
    decltype(std::chrono::high_resolution_clock::now()) last_time_;
    size_t process_bar_width_ = 50;
};

} // namespace lxrt