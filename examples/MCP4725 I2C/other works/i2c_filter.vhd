library ieee;
use ieee.std_logic_1164.all;

entity i2c_filter is
port(
	sck : in std_logic;
	sda : in std_logic;
	clk : in std_logic;
	sck_o: out std_logic;
	sda_o: out std_logic
	);
	end entity;
	
architecture rtl of i2c_filter is
signal sck_r : std_logic_vector(2 downto 0);
signal sda_r : std_logic_vector(2 downto 0);

begin

process(clk)
begin
	if rising_edge(clk) then
		sck_r(sck_r'high-1 downto 0) <= sck_r(sck_r'high-2 downto 0) & sck;
		sda_r(sda_r'high downto 0) <= sda_r(sck_r'high-1 downto 0) & sda;
		if sck_r(sck_r'high-1) = '0' and sck_r(sck_r'high-2) = '0' then
			sck_r(sck_r'high) <= '0';
		elsif sck_r(sck_r'high-1) = '1' and sck_r(sck_r'high-2) = '1' then
			sck_r(sck_r'high) <= '1';
		end if;
	end if;
end process;

sck_o <= sck_r(sck_r'high);
sda_o <= sda_r(sda_r'high);

end rtl;