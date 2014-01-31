#include "markov.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _MSC_VER
#include <Windows.h>
#define sleep(n) Sleep(n * 1000);
#endif

static unsigned int grHigh = 1;
static unsigned int grLow  = 0x4961E43;
int GRand(void) {
	grLow += (grHigh = (grHigh << 16) + (grHigh >> 16) + grLow);
	return grHigh;
}

struct markov *new_markov(unsigned char N, unsigned char M, byte *start, byte *end) {
	struct markov *result;
	result = (struct markov *)malloc(sizeof(*result));
	memset(result, 0, sizeof(*result));
	result->N     = N;
	result->M     = M;
	result->start = start;
	result->end   = end;
	return result;
}

#define _check_index(markov, a, b) (memcmp((a), (b), (markov)->N) == 0)
#define _check_value(markov, a, b) (memcmp((a), (b), (markov)->M) == 0)

static size_t _make_hash(byte *data, size_t L) {
	size_t result = 0xBADF00D;
	while(L--) {
		result = (result << 16) + ((result >> 16) ^ *data++);
	}
	return result;
}

static void _slot_enter(struct markov *markov, struct markovSlot *slot, byte *value) {
	size_t hash = _make_hash(value, markov->M);
	size_t tests;
	size_t p, np, free_p, new_pairs;
	struct markovPair *new_pair;

	++slot->total_weight;

	while(1) {
		free_p = 1 << 31;

		tests = slot->pairs;
		while(tests--) {
			p = (hash + tests) % slot->pairs;
			if(free_p == 1 << 31 && slot->pair[p].hash == 0) {
				free_p = p;
			}
			if(slot->pair[p].hash == hash && _check_value(markov, value, slot->pair[p].value)) {
				++slot->pair[p].weight;
				return;
			}
		}

		if(free_p != 1 << 31) {
			slot->pair[p].hash = hash;
			slot->pair[p].value = (byte *)malloc(markov->M);
			memcpy(slot->pair[p].value, value, markov->M);
			++slot->pair[p].weight;
			return;
		}

		new_pairs = slot->pairs ? slot->pairs << 1 : 1;
		new_pair = (struct markovPair *)malloc(sizeof(*new_pair) * new_pairs);
		memset(new_pair, 0, sizeof(*new_pair) * new_pairs);
		if(slot->pair) {
			for(p = 0; p < slot->pairs; p++) {
				tests = new_pairs;
				while(tests--) {
					np = (slot->pair[p].hash + tests) % new_pairs;
					if(new_pair[np].hash == 0) {
						memcpy(slot->pair + p, new_pair + np, sizeof(*slot->pair));
					}
				}
			}
			free(slot->pair);
		}
		slot->pair = new_pair;
		slot->pairs = new_pairs;
	}
}

static void _markov_enter(struct markov *markov, byte *index, byte *value) {
	size_t hash = _make_hash(index, markov->N);
	size_t tests;
	size_t s, ns, free_s, new_slots;
	struct markovSlot *new_slot;

	while(1) {
		free_s = 1 << 31;

		tests = markov->slots;
		while(tests--) {
			s = (hash + tests) % markov->slots;
			if(free_s == 1 << 31 && markov->slot[s].hash == 0) {
				free_s = s;
			}
			if(markov->slot[s].hash == hash && _check_index(markov, index, markov->slot[s].index)) {
				_slot_enter(markov, markov->slot + s, value);
				return;
			}
		}

		if(free_s != 1 << 31) {
			memset(markov->slot + free_s, 0, sizeof(*markov->slot));
			markov->slot[free_s].hash = hash;
			markov->slot[free_s].index = (byte *)malloc(markov->N);
			memcpy(markov->slot[free_s].index, index, markov->N);
			_slot_enter(markov, markov->slot + free_s, value);
			return;
		}

		new_slots = markov->slots ? markov->slots << 1 : 1;
		new_slot = (struct markovSlot *)malloc(sizeof(*new_slot) * new_slots);
		memset(new_slot, 0, sizeof(*new_slot) * new_slots);
		if(markov->slot) {
			for(s = 0; s < markov->slots; s++) {
				tests = new_slots;
				while(tests--) {
					ns = (markov->slot[s].hash + tests) % new_slots;
					if(new_slot[ns].hash == 0) {
						memcpy(markov->slot + s, new_slot + ns, sizeof(*new_slot));
						break;
					}
				}
			}
			free(markov->slot);
		}
		markov->slot = new_slot;
		markov->slots = new_slots;
	}
}

static void _slot_get(struct markovSlot *slot, byte **value) {
	unsigned int energy = GRand() % slot->total_weight;
	size_t offset = GRand();
	size_t p;
	while(1) {
		p = offset++ % slot->pairs;
		if(slot->pair[p].hash) {
			if(slot->pair[p].weight > energy) {
				*value = slot->pair[p].value;
				return;
			}
			energy -= slot->pair[p].weight;
		}
	}
}

static void _markov_get(struct markov *markov, byte *index, byte **value) {
	size_t hash = _make_hash(index, markov->N);
	size_t tests = markov->slots;
	size_t s;

	*value = NULL;

	while(tests--) {
		s = (hash + tests) % markov->slots;
		if(markov->slot[s].hash == hash && _check_index(markov, index, markov->slot[s].index)) {
			_slot_get(markov->slot + s, value);
			return;
		}
	}
}

void markov_build(struct markov *markov, size_t bytes, byte *value) {
	size_t offset = 0;
	byte *data;

	if(bytes == 0) {
		return;
	}

	data = (byte *)malloc(bytes + markov->M + markov->N);
	for(offset = 0; offset < markov->N; offset += markov->M) {
		memcpy(data + offset, markov->start, markov->M);
	}
	memcpy(data + offset, value, bytes);
	memcpy(data + offset + bytes, markov->end, markov->M);

	bytes += markov->M + markov->N;

	offset = 0;
	while(offset + markov->N < bytes) {
		_markov_enter(markov, data + offset, data + offset + markov->N);
		offset += markov->M;
	}

	free(data);
}

size_t markov_generate(struct markov *markov, size_t maxbytes, byte *result) {
	size_t offset = 0;
	byte *index, *value;

	index = (byte *)malloc(markov->N);
	for(offset = 0; offset < markov->N; offset += markov->M) {
		memcpy(index + offset, markov->start, markov->M);
	}
	
	for(offset = 0; offset < maxbytes; offset += markov->M) {
		_markov_get(markov, index, &value);
		memcpy(result, value, markov->M);
		result += markov->M;
		if(_check_value(markov, value, markov->end)) {
			goto cleanup;
		}
		memmove(index, index + markov->M, markov->N - markov->M);
		memcpy(index + markov->N - markov->M, value, markov->M);
	}

cleanup:
	free(index);
	return offset;
}

size_t markov_memory(struct markov *markov) {
	size_t result = 0, s, p;
	for(s = 0; s < markov->slots; s++) {
		if(markov->slot[s].hash == 0) {
			continue;
		}
		for(p = 0; p < markov->slot[s].pairs; p++) {
			if(markov->slot[s].pair[p].hash == 0) {
				continue;
			}
			result += markov->M;
		}
		result += markov->N;
		result += sizeof(*markov->slot[s].pair) * markov->slot[s].pairs;
	}
	result += sizeof(*markov->slot) * markov->slots;
	result += sizeof(*markov);
	return result;
}

void markov_free(struct markov *markov) {
	size_t s, p;
	for(s = 0; s < markov->slots; s++) {
		if(markov->slot[s].hash == 0) {
			continue;
		}
		for(p = 0; p < markov->slot[s].pairs; p++) {
			if(markov->slot[s].pair[p].hash == 0) {
				continue;
			}
			free(markov->slot[s].pair[p].value);
		}
		free(markov->slot[s].index);
		free(markov->slot[s].pair);
	}
	free(markov->slot);
	free(markov);
}

#ifdef TEST_MARKOV
#include <stdio.h>

char *test_text = "Theeghkal Suemgar Yuomat Thiathet Nysumche Slacksam Rakoldough Aringon Foenbur Isuskir Itosas Phildach Dutaidyn Nismor Lyrilnys Chaburen Dandan Guhatdel Athchesay Llaolddra Chomcha Schohen Llayight Whaldrak Taitanath Tinrothem Essissine Nyageelm Ruinque Nabos Ardsultia Chreskel Naneng Smoadhat Rodirque Lohonris Zhaysswor Shoellkal Siachund Shiashril Zoangech Gozale Bytaner Civorcha Cerghaser Koessque Raulrod Banskeliss Nysawaugh Radnalund Taslerran Liangend Loseror Liemtor Omadit Dynumper Denurnny Sneawen Trieckaugh Huindight Keashest Ightaughdra Rakrodver Nalskelryn Dyannal Briquaunt Lalest Syngar Snaesden Lisem Theadver Ashraktor Yaerdurn Yiendach Quyrque Warhintin Smealny Maucrod Nairtia Chaelyer Beiawor Brooqat Polagehin Therengech Liashray Erhatler Hysulunt Skelmorris Echemdan Angshytin Nalraken Douhar Whorayine Oughtorar Chadelina Jydild Llivorpol Denoraugh Iainggha Rhelerit Phoitban Raynalver Hohica Atiagei Burese Elluimy Vereri Ishyawy Itose Oriligaro Yelilopi Andiru Behyroro Osseyly Vohey Lynune Angughai Lanikuma Karuhepu Yocafabu Catiko Leyuhigi Marier Bifar Oshyithera Iestiaugho Arodaworo Timuni Yelmobure Cehor Aemulera Ciryc Womey Uthili Kanale Sonaca Sudein Unili Yshiarry Syniso Yrouta Tuday Ryhil Lawiv Caxorevu Buveyst Zigad Rylute Lelesaha Oataso Bidamo Elororayo Oheve Talob Walul Ightoceru Nunir Moreed Etasiwore Ykelauske Osheinu Nasesy Fohiro Naxon Rezidexo Hykatitu Cikyny Dabim Nikibaro Zideash Iorechao Uleridela Dyjig Teges Ruryti Todalazy Ainaymosu Supit Laraud Tisewi Wonirizi Kenein Sabud Aumioldo Irakusto Laleyl Rovel Lubulado Kohok Irremu Nomesele Yiloot Ephayle Jyrexage Yrakogare Uranuemi Nojig Tilut Taxisahy Tibin Nodij Kejanu Oaleaoldi Eghauhini Itonuskhinu Bankimnysroth Radestosiamos Charothdynskelbel Ildtankimsay Shyemserortor Verangataldche Taiuskangdelor Hinkelkinny Seraughachannal  elquetiashyvor Inahondenoughu Radraydynusku Onashumdary Tonurndelough Itdariroughi Askimettinis Lerardrothveru Belrodperturu Iranmosomche Ornyirwarshy Burvesdynhonaugh Hatustomust Honadsamquee Raytasadwore Ingangerim Cerenleremton Yerwardragedyn Enworunttiao Kelardadangi Anestuskacku Athinerilghae Honloranildi Ardartiarilo Rodvortasrayi Ackmornyquekel Banemkaltoner Iasamnyadmor Kelachiaene Ristiawdelu Alddynvoressa Draustisadrak Adenaughtano Aldenhinit Cernalachseru Rayitdelsayold Nydarturalever Adrilmosler Danusthonnys Ustquakelere Nallerqueengald Dipertle Chaozard Nysochu Cheortle Sneezeuzard Quaozard Ashymon Boneymon Iaozard Dipartle Dumbichu Ackuchu Meatezard Bumpamon Ghauchu Ustazard Wipeemon Meatochu Taiozard Torychu Athezard Doofichu Puffyzard Bumamon Estazard Denymon Headyrtle Onizard Orizard Lunkezard Beefechu Itechu Thimbleozard Faceazard Issomon Omimon Chachu Clotazard Dumbochu Rakimon Shyochu Enthortle Ememon Omurtle Knuckleamon onichu Irizard Untazard Ghaezard Serozard Knuckleertle";

int main(int argc, char *argv[]) {
	struct markov *markov = new_markov(3, 1, (byte *)"\1", (byte *)"\0");
	char result[50];
	char *text = test_text, *end;
	do {
		end = strchr(text, ' ');
		if(end) {
			markov_build(markov, end - text, (byte *)text);
			text = end + 1;
		} else {
			markov_build(markov, strlen(text), (byte *)text);
		}
	} while(end);
	printf("%f kb for %f kb of text\n", markov_memory(markov) / 1024.0, strlen(test_text) / 1024.0);
	grHigh = time(NULL);
	while(1) {
		markov_generate(markov, 50, (byte *)result);
		printf("%-79s\r", result);
		sleep(0.2);
	}
	markov_free(markov);
}
#endif
