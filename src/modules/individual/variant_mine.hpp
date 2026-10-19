#pragma once
// ============================================================================
// ВАРИАНТ 15: Warehouse TCO Calculator (совокупная стоимость владения складом)
//
// Имя namespace должно остаться TaskVariant --- так работает task.hpp.
// ============================================================================

#include <string>
#include <cmath>
#include "../../json.hpp"

namespace TaskVariant {

    // ────────────────────────────────────────────────────────────────────────
    // Структура входных данных. Соответствует полям JSON-запроса.
    // ────────────────────────────────────────────────────────────────────────
    struct Input {
        double rent_square_meter_cost;              // тариф за кв.м
        double total_area;                          // площадь, кв.м
        double electricity_and_utility_monthly;     // коммуналка/мес
        double staff_salary_fund;                   // ФОТ/мес
        double average_pallet_throughput_monthly;   // паллетооборот/мес
    };

    // ────────────────────────────────────────────────────────────────────────
    // Структура выходных данных. Соответствует полям JSON-ответа.
    // ────────────────────────────────────────────────────────────────────────
    struct Output {
        double monthly_fixed_costs;         // постоянные затраты в месяц
        double storage_cost_per_pallet;     // стоимость хранения паллеты
        double break_even_throughput;       // точка безубыточности по паллетам
    };

    // ────────────────────────────────────────────────────────────────────────
    // Проверка входных данных.
    // Возвращает "" если всё ок, иначе --- текст ошибки.
    // ────────────────────────────────────────────────────────────────────────
    inline std::string validate(const Input& in) {
        if (in.rent_square_meter_cost <= 0.0)
            return "rent_square_meter_cost must be > 0.0";
        if (in.total_area < 10.0 || in.total_area > 500000.0)
            return "total_area must be in [10.0, 500000.0]";
        if (in.electricity_and_utility_monthly < 0.0)
            return "electricity_and_utility_monthly must be >= 0.0";
        if (in.staff_salary_fund < in.total_area * 0.5)
            return "staff_salary_fund must be >= total_area * 0.5";
        if (in.average_pallet_throughput_monthly < 0.0)
            return "average_pallet_throughput_monthly must be >= 0.0";
        return "";
    }

    // ────────────────────────────────────────────────────────────────────────
    // Основные расчёты.
    // ────────────────────────────────────────────────────────────────────────
    inline Output calculate(const Input& in) {
        Output out{};

        // Постоянные затраты в месяц
        out.monthly_fixed_costs =
            (in.rent_square_meter_cost * in.total_area)
            + in.electricity_and_utility_monthly
            + in.staff_salary_fund;

        // Стоимость хранения одной паллеты
        out.storage_cost_per_pallet =
            (in.average_pallet_throughput_monthly > 0.0)
                ? (out.monthly_fixed_costs / in.average_pallet_throughput_monthly)
                : out.monthly_fixed_costs;

        // Точка безубыточности (при условном доходе 15 за паллету)
        out.break_even_throughput = out.monthly_fixed_costs / 15.0;

        return out;
    }

    // ────────────────────────────────────────────────────────────────────────
    // Разбор JSON-тела запроса в Input.
    // ────────────────────────────────────────────────────────────────────────
    inline Input parse(const std::string& body) {
        Input in{};
        auto get_d = [&](const std::string& key) -> double {
            auto p = body.find("\"" + key + "\"");
            if (p == std::string::npos) return 0.0;
            p = body.find(':', p);
            if (p == std::string::npos) return 0.0;
            try { return std::stod(body.substr(p + 1)); }
            catch (...) { return 0.0; }
        };
        in.rent_square_meter_cost            = get_d("rent_square_meter_cost");
        in.total_area                        = get_d("total_area");
        in.electricity_and_utility_monthly   = get_d("electricity_and_utility_monthly");
        in.staff_salary_fund                 = get_d("staff_salary_fund");
        in.average_pallet_throughput_monthly = get_d("average_pallet_throughput_monthly");
        return in;
    }

    // ────────────────────────────────────────────────────────────────────────
    // Формирование JSON-ответа из Output.
    // ────────────────────────────────────────────────────────────────────────
    inline std::string to_json(const Output& out) {
        JObj o;
        o.num("monthly_fixed_costs",     out.monthly_fixed_costs);
        o.num("storage_cost_per_pallet", out.storage_cost_per_pallet);
        o.num("break_even_throughput",   out.break_even_throughput);
        return o.str();
    }

} // namespace TaskVariant