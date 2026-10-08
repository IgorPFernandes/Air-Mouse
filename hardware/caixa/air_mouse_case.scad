// Air Mouse — caixa do chaveiro (formato B) para Seeed XIAO ESP32-C6
// Unidades: mm. Abra no OpenSCAD (gratuito), escolha a "peca" no Customizer,
// renderize (F6) e exporte o STL (F7). Imprima base, tampa e bandeja separadas.
//
// Eixos: X = largura (esquerda -> direita), Y = comprimento (frente -> tras),
// Z = altura. A origem e o canto interno frente-esquerda, em cima do fundo.
// A frente tem o laser e o LED IR; a traseira tem o USB-C da XIAO e o rasgo
// da argola do chaveiro (canto traseiro de baixo, a direita).

/* [Peca] */
peca = "montagem"; // [montagem, explodida, base, tampa, bandeja]

/* [Bateria com o TP4056 colado - MEDIR E AJUSTAR] */
bat_larg = 30;    // largura da bateria (X)
bat_comp = 30;    // comprimento da bateria (Y), da ponta da protecao ate o lado do USB
bat_esp  = 5.0;   // espessura da bateria (sem o TP4056)
tp_larg  = 17.5;  // largura da placa TP4056
tp_comp  = 28;    // comprimento da placa TP4056
tp_recuo = 0;     // quanto o USB do TP4056 fica para dentro da borda da bateria
abrir_usb_tp4056 = false; // abre um 2o furo USB-C para carregar pelo TP4056

/* [Placas] */
xiao_larg = 17.8;
xiao_comp = 21;
xiao_pcb  = 1.2;
mpu_larg  = 16.0; // GY-521, sem os pinos
mpu_comp  = 21.0;

/* [Acabamento] */
argola  = true;   // passagem escondida para argola de chaveiro
estrias = true;   // estrias laterais para a mao nao escorregar

/* [Caixa] */
parede  = 2.5;    // mais grossa para aguentar os cantos bem arredondados
fundo   = 2.0;
t_tampa = 2.0;
r_canto = 4;      // raio de TODAS as quinas (deixa a caixa "fofinha")
folga   = 0.15;   // folga da aba da tampa
explodir = 12;    // afastamento entre camadas na vista explodida

$fn = 48;

// ---------- medidas derivadas ----------
z_bat     = bat_esp + 0.5;          // topo da bateria (0,5 mm para ela poder estufar)
z_tp_topo = z_bat + 1.6 + 3.2;      // topo do conector USB do TP4056
z_bandeja = z_tp_topo + 0.7;        // fundo da bandeja das placas
z_placas  = z_bandeja + 1.0;        // XIAO e MPU apoiam aqui
int_h     = z_placas + 8.5;         // altura interna (ate a tampa)

pocket_mpu  = mpu_larg + 0.4;
pocket_xiao = xiao_larg + 0.4;
x_aba  = pocket_mpu;                // divisoria entre MPU e XIAO
x_xiao = x_aba + 0.8;
comp_placas = max(mpu_comp, xiao_comp) + 0.4;

int_w = max(pocket_mpu + 0.8 + pocket_xiao, bat_larg + 5.4);
int_l = max(bat_comp + 2.0, comp_placas + 10.6);
y_bandeja = int_l - comp_placas;

ext_w = int_w + 2 * parede;
ext_l = int_l + 2 * parede;
ext_h = fundo + int_h + t_tampa;

bat_y1 = int_l - 0.2;
bat_y0 = bat_y1 - bat_comp;
tp_y1  = bat_y1 - tp_recuo;
tp_y0  = tp_y1 - tp_comp;
tp_usb_x = tp_larg / 2;
tp_usb_z = z_bat + 1.6 + 1.6;

// laser (6 mm) e LED IR (3 mm) deitados na faixa livre da bateria, ao lado do TP4056,
// na mesma altura e com 1,2 mm de parede entre os dois furos
laser_x = tp_larg + 4;
laser_z = z_bat + 3;
ledir_d = 3;
ledir_x = laser_x + 6.1;
ledir_z = laser_z;

xiao_usb_x = x_xiao + pocket_xiao / 2;
xiao_usb_z = z_placas + xiao_pcb + 1.6;

cx = int_w / 2;
// 1 = liga/laser, 2 = clique, 3 = centro/velocidade, 4 = projetor
botoes = [[cx, 4.2], [cx, 14], [cx - 9.5, 21.5], [cx + 9.5, 21.5]];
rec_x = 6;   rec_y = 5;     // receptor IR deitado, lente para cima
led_x = int_w - 5.4; led_y = 6;  // LED RGB 3 mm

// passagem da argola: um rasgo estreito no canto traseiro de baixo (lado direito),
// aberto para tras e para baixo, com uma barrinha escondida dentro onde a argola
// abraca. Por dentro, um bolsinho fechado separa a argola da eletronica.
// Medido para argolas de ate 30 mm com arame de 2 mm.
bolso_x0  = int_w - 5.0;
rasgo_x0  = int_w - 4.2;
rasgo_larg = 3.4;
rasgo_comp = 7.5;   // quanto o rasgo avanca por baixo, a partir da parede de tras
rasgo_alt  = 6.0;   // altura do rasgo na traseira, a partir do fundo
barra_d    = 3.5;
barra_y    = int_l - 0.4;
barra_z    = 0.4;

assert(bat_larg - tp_larg >= 12, "A faixa livre ao lado do TP4056 precisa de 12 mm para o laser e o LED IR");
assert(y_bandeja >= 10.5, "Falta espaco na frente para o laser (10,5 mm)");
assert(!argola || bolso_x0 >= bat_larg + 0.3, "Sem espaco ao lado da bateria para a passagem da argola");

// ---------- utilitarios ----------
module casca() {
    // caixa inteira com todas as quinas arredondadas; base e tampa sao cortadas dela
    translate([-parede, -parede, -fundo]) hull()
        for (x = [r_canto, ext_w - r_canto], y = [r_canto, ext_l - r_canto], z = [r_canto, ext_h - r_canto])
            translate([x, y, z]) sphere(r = r_canto);
}
module cil_y(x, y, z, d, h) {
    translate([x, y, z]) rotate([-90, 0, 0]) cylinder(d = d, h = h);
}
module rasgo_y(w, h, comp) {
    hull() for (dx = [-(w - h) / 2, (w - h) / 2]) translate([dx, 0, 0]) cylinder(d = h, h = comp);
}
module furo_usb(x, z) {
    translate([x, int_l - 1, z]) rotate([-90, 0, 0]) rasgo_y(9.4, 3.6, parede + 2);
    // rebaixo externo para a capa do cabo chegar perto do conector (sobram 0,6 mm de parede)
    translate([x, int_l + 0.6, z]) rotate([-90, 0, 0]) rasgo_y(12.6, 7.0, parede + 1);
}

// ---------- base ----------
module ressaltos() {
    // apoios da bandeja nas paredes laterais, com chanfro de 45 graus (imprime sem suporte)
    hull() {
        translate([0, y_bandeja, z_bandeja - 1]) cube([1.2, int_l - y_bandeja, 1]);
        translate([0, y_bandeja, z_bandeja - 2.2]) cube([0.01, int_l - y_bandeja, 0.01]);
    }
    hull() {
        translate([int_w - 1.2, y_bandeja, z_bandeja - 1]) cube([1.2, int_l - y_bandeja, 1]);
        translate([int_w - 0.01, y_bandeja, z_bandeja - 2.2]) cube([0.01, int_l - y_bandeja, 0.01]);
    }
}
module estrias_cortes() {
    for (zz = [3 : 2.5 : 15.5]) {
        translate([-parede - 1, 3, zz - 0.5]) cube([1.5, int_l - 6, 1]);
        translate([int_w + parede - 0.5, 3, zz - 0.5]) cube([1.5, int_l - 6, 1]);
    }
}
module argola_rasgo() {
    translate([rasgo_x0, int_l - rasgo_comp, -fundo - 1])
        cube([rasgo_larg, rasgo_comp + parede + 1, fundo + 1 + rasgo_alt]);
}
module bolso_argola() {
    y0 = int_l - rasgo_comp - 0.8;
    h  = rasgo_alt + 0.8;
    translate([bolso_x0, y0, 0])            cube([5.0, 0.8, h]);                  // parede da frente
    translate([bolso_x0, y0, 0])            cube([0.8, int_l - y0, h]);           // parede do lado da bateria
    translate([int_w - 0.8, y0, 0])         cube([0.8, int_l - y0, h]);           // parede do lado de fora
    translate([bolso_x0, y0, rasgo_alt])    cube([5.0, int_l - y0, 0.8]);         // teto
    // barrinha onde a argola abraca (imprime como ponte, sem suporte)
    translate([rasgo_x0 - 0.01, barra_y, barra_z]) rotate([0, 90, 0]) cylinder(d = barra_d, h = rasgo_larg + 0.02);
}
module base() {
    difference() {
        intersection() {
            casca();
            translate([-50, -50, -50]) cube([200, 200, 50 + int_h]);
        }
        cube([int_w, int_l, int_h + 1]);
        cil_y(laser_x, -parede - 1, laser_z, 6.4, parede + 2);   // laser
        cil_y(ledir_x, -parede - 1, ledir_z, ledir_d + 0.3, parede + 2);   // LED IR
        furo_usb(xiao_usb_x, xiao_usb_z);
        if (abrir_usb_tp4056) furo_usb(tp_usb_x, tp_usb_z);
        if (estrias) estrias_cortes();
        if (argola) argola_rasgo();
    }
    ressaltos();
    if (argola) bolso_argola();
}

// ---------- bandeja (segura XIAO e MPU firmes) ----------
module bandeja() {
    difference() {
        union() {
            translate([0.2, y_bandeja, z_bandeja]) cube([int_w - 0.4, int_l - y_bandeja - 0.2, 1.0]);
            translate([x_aba, y_bandeja, z_placas]) cube([0.8, int_l - y_bandeja - 0.2, 1.0]);
        }
        // janela sob a XIAO para os fios dos pads de baixo (BAT+, BAT-, GPIO4-7)
        translate([x_xiao + 3, y_bandeja + 3, z_bandeja - 1]) cube([pocket_xiao - 6, comp_placas - 6.5, 3]);
        // passagem de fios ao lado do MPU (o resto fica inteiro para colar o MPU)
        translate([3, y_bandeja + 1.5, z_bandeja - 1]) cube([pocket_mpu - 6, 2.5, 3]);
    }
}

// ---------- tampa ----------
module tampa() {
    difference() {
        union() {
            intersection() {
                casca();
                translate([-50, -50, int_h]) cube([200, 200, 50]);
            }
            // aba que encaixa por dentro das paredes
            translate([folga, folga, int_h - 2]) difference() {
                cube([int_w - 2 * folga, int_l - 2 * folga, 2]);
                translate([0.8, 0.8, -1]) cube([int_w - 2 * folga - 1.6, int_l - 2 * folga - 1.6, 4]);
            }
            // soquetes dos botoes 6x6 (o corpo do botao encaixa por baixo)
            for (b = botoes) translate([b[0] - 3.95, b[1] - 3.95, int_h - 2.5]) cube([7.9, 7.9, 2.5]);
            // soquete do receptor IR
            translate([rec_x - 3.6, rec_y - 4.2, int_h - 2.5]) cube([7.2, 8.4, 2.5]);
        }
        for (b = botoes) {
            translate([b[0] - 3.15, b[1] - 3.15, int_h - 3]) cube([6.3, 6.3, 3]);
            translate([b[0], b[1], int_h - 1]) cylinder(d = 4.0, h = t_tampa + 2);
        }
        translate([rec_x - 2.9, rec_y - 3.5, int_h - 3]) cube([5.8, 7.0, 3]);
        translate([rec_x, rec_y, int_h - 1]) cylinder(d = 5, h = t_tampa + 2);
        translate([led_x, led_y, int_h - 1]) cylinder(d = 3.1, h = t_tampa + 2);
        // marcas para achar no tato: anel no botao 1, ponto ao lado do botao 4
        translate([botoes[0][0], botoes[0][1], int_h + t_tampa - 0.4]) difference() {
            cylinder(d = 8, h = 1);
            translate([0, 0, -1]) cylinder(d = 6.6, h = 3);
        }
        translate([botoes[3][0] + 5.5, botoes[3][1], int_h + t_tampa - 0.4]) cylinder(d = 1.6, h = 1);
    }
}

// ---------- componentes (so para visualizar) ----------
module botao_6x6(x, y) {
    color("dimgray") translate([x - 3, y - 3, int_h - 3.5]) cube([6, 6, 3.5]);
    color("gray") translate([x, y, int_h]) cylinder(d = 3.5, h = 3.5);
}
module componentes(e = 0) {
    color("silver") translate([0, bat_y0, 0]) cube([bat_larg, bat_comp, bat_esp]);
    translate([0, 0, e]) {
        color("royalblue") translate([0, tp_y0, z_bat]) cube([tp_larg, tp_comp, 1.6]);
        color("gainsboro") translate([tp_usb_x - 4.45, tp_y1 - 7.4, z_bat + 1.6]) cube([8.9, 7.4, 3.2]);
        color("firebrick") cil_y(laser_x, 0, laser_z, 6, 10.5);
        color("darkslateblue") cil_y(ledir_x, 0, ledir_z, ledir_d, 5.3);
        color("forestgreen") translate([tp_larg + 0.5, 12, z_bat]) cube([11.5, 15, 1.6]);
    }
    translate([0, 0, 2 * e]) color("lightgray") bandeja();
    translate([0, 0, 3 * e]) {
        color("teal") translate([0.2, int_l - mpu_comp, z_placas]) cube([mpu_larg, mpu_comp, 1.6]);
        color("mediumpurple") translate([x_xiao + 0.2, int_l - xiao_comp, z_placas]) cube([xiao_larg, xiao_comp, xiao_pcb]);
        color("gainsboro") translate([xiao_usb_x - 4.45, int_l - 7.4, z_placas + xiao_pcb]) cube([8.9, 7.4, 3.2]);
    }
    translate([0, 0, 4 * e]) {
        for (b = botoes) botao_6x6(b[0], b[1]);
        color("black") translate([rec_x - 2.8, rec_y - 3.4, int_h - 4.5]) cube([5.6, 6.8, 4.5]);
        color("white") translate([led_x, led_y, int_h - 4]) cylinder(d = 3, h = 5.3);
    }
}

// ---------- saida ----------
if (peca == "base") {
    base();
} else if (peca == "tampa") {
    translate([0, 0, int_h + t_tampa]) rotate([180, 0, 0]) tampa();  // imprime de cabeca para baixo
} else if (peca == "bandeja") {
    translate([0, 0, -z_bandeja]) bandeja();
} else {
    e = (peca == "explodida") ? explodir : 0;
    color("white", 0.25) base();
    componentes(e);
    translate([0, 0, 5 * e]) color("white", 0.25) tampa();
}

echo(str("Caixa externa: ", ext_w, " x ", ext_l, " x ", ext_h, " mm"));
