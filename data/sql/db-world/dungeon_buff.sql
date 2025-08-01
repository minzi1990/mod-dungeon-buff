-- Add SPELL_ATTR0_CU_AURA_CANNOT_BE_SAVED
DELETE FROM `spell_custom_attr` WHERE `spell_id` IN (90000, 90001);
INSERT INTO `spell_custom_attr` (`spell_id`, `attributes`) VALUES
(90000, 0x01000000),
(90001, 0x01000000);
