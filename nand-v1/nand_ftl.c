#include "nand_ftl.h"
#include "esp_log.h"

// Si más adelante creamos nand_fsm_program_page() y nand_fsm_read_page()
// solo hay que declarar los prototipos aquí para compilar ya:
bool nand_fsm_program_page(const uint8_t *src, uint16_t block, uint8_t page);
bool nand_fsm_read_page(uint8_t *dest, uint16_t block, uint8_t page);

static const char *TAG = "FTL";

// Tabla de mapeo LBA → dirección física
static ftl_entry_t ftl_table[FTL_MAX_LBAS];

// Control del siguiente espacio libre
static uint16_t next_block = 0;
static uint8_t next_page = 0;

bool ftl_init(void) {
    for (int i = 0; i < FTL_MAX_LBAS; i++) {
        ftl_table[i].valid = false;
    }
    next_block = 0;
    next_page = 0;
    ESP_LOGI(TAG, "FTL inicializada");
    return true;
}

bool ftl_get_mapping(uint16_t lba, uint16_t *block, uint8_t *page) {
    if (lba >= FTL_MAX_LBAS || !ftl_table[lba].valid) return false;
    *block = ftl_table[lba].block;
    *page = ftl_table[lba].page;
    return true;
}

bool ftl_write_lba(uint16_t lba, const uint8_t *src) {
    if (lba >= FTL_MAX_LBAS) return false;

    if (next_block >= FTL_TOTAL_BLOCKS) {
        ESP_LOGE(TAG, "No hay bloques disponibles");
        return false;
    }

    if (!nand_fsm_program_page(src, next_block, next_page)) {
        ESP_LOGE(TAG, "Error escribiendo bloque=%d página=%d", next_block, next_page);
        return false;
    }

    ftl_table[lba] = (ftl_entry_t){ .block = next_block, .page = next_page, .valid = true };

    // Avanza a la siguiente posición libre
    next_page++;
    if (next_page >= FTL_PAGES_PER_BLOCK) {
        next_page = 0;
        next_block++;
    }

    return true;
}

bool ftl_read_lba(uint16_t lba, uint8_t *dest) {
    if (lba >= FTL_MAX_LBAS || !ftl_table[lba].valid) return false;
    return nand_fsm_read_page(dest, ftl_table[lba].block, ftl_table[lba].page);
}

bool ftl_erase_block(uint16_t lba) {
    if (lba >= FTL_MAX_LBAS || !ftl_table[lba].valid) {
        ESP_LOGW("FTL", "LBA %d inválido o no mapeado aún", lba);
        return false;
    }

    uint16_t block = ftl_table[lba].block;
    ESP_LOGI("FTL", "Borrando bloque físico %d asociado a LBA %d", block, lba);

    return nand_fsm_block_erase(block);
}