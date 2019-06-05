#include <cassert>
#include <stdexcept>

#include "npstat/nm/GaussHermiteQuadrature.hh"

static const unsigned allowed[] = {16U, 32U, 64U, 100U, 128U, 256U};

static const long double x16[8] = {
    0.2734810461381524521582804L, 0.8229514491446558925824545L,
    1.38025853919888079637209L,   1.951787990916253977434655L,
    2.546202157847481362159329L,  3.176999161979956026813995L,
    3.869447904860122698719424L,  4.688738939305818364688499L
};

static const long double w16[8] = {
    0.5079294790166137419135173L,    0.2806474585285336753694633L,
    0.08381004139898582941542073L,   0.0128803115355099736834643L,
    0.0009322840086241805299142773L, 0.00002711860092537881512018914L,
    2.320980844865210653387494e-7L,  2.654807474011182244709264e-10L
};

static const long double x32[16] = {
    0.1948407415693993267087413L, 0.5849787654359324484669575L,
    0.9765004635896828384847049L, 1.370376410952871838161706L,
    1.767654109463201604627673L,  2.169499183606112173305706L,
    2.577249537732317454030929L,  2.992490825002374206285494L,
    3.417167492818570735873927L,  3.853755485471444643887873L,
    4.305547953351198445263487L,  4.777164503502596393035794L,
    5.27555098651588012781906L,   5.812225949515913832765966L,
    6.409498149269660412173764L,  7.125813909830727572795208L
};

static const long double w32[16] = {
    0.3752383525928023928668184L,    0.2774581423025298981376989L,
    0.1512697340766424825751471L,    0.06045813095591261418658576L,
    0.01755342883157343030343784L,   0.003654890326654428079125657L,
    0.0005362683655279720459702381L, 0.00005416584061819982558001939L,
    3.650585129562376057370324e-6L,  1.574167792545594029268693e-7L,
    4.098832164770896618235041e-9L,  5.933291463396638614511568e-11L,
    4.215010211326447572969445e-13L, 1.197344017092848665828682e-15L,
    9.23173653651829223349442e-19L,  7.310676427384162393274278e-23L
};

static const long double x64[32] = {
    0.1383022449870097241150498L, 0.4149888241210786845769291L,
    0.6919223058100445772682193L, 0.9692694230711780167435415L,
    1.247200156943117940693565L,  1.52588914020986366294897L,
    1.805517171465544918903774L,  2.086272879881762020832563L,
    2.368354588632401404111511L,  2.651972435430635011005458L,
    2.937350823004621809685339L,  3.224731291992035725848171L,
    3.514375935740906211539951L,  3.806571513945360461165972L,
    4.101634474566656714970981L,  4.399917168228137647767933L,
    4.701815647407499816097538L,  5.007779602198768196443703L,
    5.31832522463327085732365L,   5.63405216434997214724992L,
    5.955666326799486045344567L,  6.284011228774828235418093L,
    6.62011226263602737903666L,   6.965241120551107529242642L,
    7.321013032780949201189569L,  7.689540164040496828447804L,
    8.073687285010225225858791L,  8.477529083379863090564166L,
    8.907249099964769757295973L,  9.373159549646721162545652L,
    9.895287586829539021204461L,  10.52612316796054588332683L
};

static const long double w64[32] = {
    0.2713774249413039779456065L,     0.232994786062678046650566L,
    0.171685842349083702000728L,      0.1084983493061868406330258L,
    0.05873998196409943454968895L,    0.02720312895368891845383482L,
    0.01075604050987913704946517L,    0.003622586978534458760668125L,
    0.001036329099507577663456742L,   0.000250983698513062486082362L,
    0.00005125929135786274660821911L, 8.788499230850359181444047e-6L,
    1.258340251031184576157842e-6L,   1.495532936727247061102462e-7L,
    1.465125316476109354926622e-8L,   1.173616742321549343542506e-9L,
    7.615217250145451353315296e-11L,  3.959177766947723927236446e-12L,
    1.628340730709720362084307e-13L,  5.218623726590847522957809e-15L,
    1.280093391322438041639563e-16L,  2.351884710675819116957676e-18L,
    3.152254566503781416121347e-20L,  2.982862784279851154478701e-22L,
    1.911706883300642829958457e-24L,  7.861797788925910369099991e-27L,
    1.929103595464966850301969e-29L,  2.549660899112999256604767e-32L,
    1.557390624629763802309335e-35L,  3.421138011255740504327222e-39L,
    1.679747990108159218666288e-43L,  5.535706535856942820575463e-49L
};

static const long double x100[50] = {
    0.1107958724224394828875599L, 0.3324146923422318070458988L,
    0.5541148235916169882329977L, 0.7759507615401457819750658L,
    0.9979774360981052439241031L, 1.220250391218953058820493L,
    1.442825970215932787702578L,  1.665761508741509469866654L,
    1.88911553742700837149364L,   2.112947996371187952029771L,
    2.337320463906878505012623L,  2.562296402372608025056025L,
    2.787941423981989313190368L,  3.014323580331155516715165L,
    3.241513679631012950358773L,  3.46958563641858916976816L,
    3.698616859318491939796954L,  3.928688683427670972009502L,
    4.159886855131030540068041L,  4.392302078682684016744776L,
    4.626030635787155773074038L,  4.861175091791210210046067L,
    5.097845105089136247000092L,  5.336158360138360497277115L,
    5.576241649329924103303485L,  5.81823213520351704736227L,
    6.062278832614302638665185L,  6.308544361112135121638817L,
    6.557207031921539315980143L,  6.808463352858796414478981L,
    7.062531060248865437465523L,  7.319652822304535316332062L,
    7.580100807857488884285809L,  7.84418238446082116879208L,
    8.112247311162791917211692L,  8.384696940416265075086016L,
    8.661996168134517714376075L,  8.944689217325474478798597L,
    9.233420890219161550477937L,  9.528965823390114804697066L,
    9.832269807777969094355162L,  10.14450994129284546988859L,
    10.46718542134281214178313L,  10.80226075368471459482167L,
    11.15240438558512526489904L,  11.52141540078703024169422L,
    11.9150619431141658019848L,   12.34296422285967429510274L,
    12.82379974948780890633913L,  13.40648733814491013849802L
};

static const long double w100[50] = {
    0.2188926295874391250627106L,     0.1984628502541864777089007L,
    0.1631300305027829414192852L,     0.1215379868441041819836584L,
    0.08205182739122446468042594L,    0.05017581267742869569787715L,
    0.02777912738593351427005737L,    0.01391566522023180641788101L,
    0.006303000285608052549385533L,   0.002579273260059090173395552L,
    0.0009526921885486191174785274L,  0.0003172919710433003055500797L,
    0.00009517162778550966470160633L, 0.00002567615938454906305610276L,
    6.221524817777863317350848e-6L,   1.35179715911036728660027e-6L,
    2.629097483753725079341014e-7L,   4.568127508484939513556966e-8L,
    7.075857283889572907380399e-9L,   9.747921253871621245758587e-10L,
    1.19130063492907294981858e-10L,   1.28790382573155823281197e-11L,
    1.227878514410124970026169e-12L,  1.028874937350992546798209e-13L,
    7.548896877915243292268832e-15L,  4.829835321703033347677164e-16L,
    2.682492164760376080003197e-17L,  1.286832921121153275751754e-18L,
    5.302316183131848685301362e-20L,  1.864997675130252258165612e-21L,
    5.561026961659167317149952e-23L,  1.39484152606876708048011e-24L,
    2.917350072629332417799015e-26L,  5.037791166213187784233353e-28L,
    7.101812226384934229656621e-30L,  8.067434278709377173709162e-32L,
    7.274572596887767574509963e-34L,  5.116232604385222180439519e-36L,
    2.74878488435711249198534e-38L,   1.100470682714223669456485e-40L,
    3.185217877835917930677973e-43L,  6.420725205348472482880041e-46L,
    8.597563954825271610021206e-49L,  7.191529463463371029600923e-52L,
    3.459477936475550444644499e-55L,  8.518883081761633786654344e-59L,
    9.019222303693556179721971e-63L,  3.083028990003274811977776e-67L,
    1.972860574879452554487221e-72L,  5.908067865031206815268855e-79L
};

static const long double x128[64] = {
    0.09798382195581895431377132L, 0.2939661103002957028133519L, 
    0.4899923604154589180890444L,  0.6860919752173348720452864L, 
    0.8822945007929814060005083L,  1.078629684810908930471008L, 
    1.275127536089158321432511L,   1.471818385674486000678376L, 
    1.668732949803723630486601L,   1.865902395140598696649124L, 
    2.063358406708565977681751L,   2.261133258973062280284208L, 
    2.459259890565739401936776L,   2.657771983189483996310816L, 
    2.856704045297405282651849L,   3.056091501202680055957841L, 
    3.255970786350659346652906L,   3.456379449571737482209434L, 
    3.657356263235308096230587L,   3.858941342344281826590627L, 
    4.061176273749272824277548L,   4.26410425682551915674979L, 
    4.467770257148582683446318L,   4.672221174932638922145675L, 
    4.877506030264814412167552L,   5.083676167489339906735054L, 
    5.290785481477179576436742L,   5.498890668973909484522189L, 
    5.708051508768086261774909L,   5.918331175085811675116817L, 
    6.129796589422162024620596L,   6.34251881700177947172939L, 
    6.556573515264482889625789L,   6.772041443255928858206886L, 
    6.989009042644774011852237L,   7.207569103387333854417799L, 
    7.427821529952301115657396L,   7.64987422768100656113185L, 
    7.873844133535434466787109L,   8.099858421507896075457943L, 
    8.32805592079014664500802L,    8.558588795064508288960304L, 
    8.791624544888686406350403L,   9.027348413394788344826656L, 
    9.26596630029617592185364L,    9.507708323279056576954902L, 
    9.752833213439168674549426L,   10.00163379893012284601114L, 
    10.25444392847093071702454L,   10.51164732991486861739414L, 
    10.77368911516144067131166L,   11.0410909760196333842429L, 
    11.314471644289977917212L,     11.59457505474145174678208L, 
    11.88231011887831158083592L,   12.17880861983124631327407L, 
    12.48551258534944816069906L,   12.80431208206713129501371L, 
    13.13777478802765110106506L,   13.48955641262314182637912L, 
    13.86520698447624158977684L,   14.27398130478783556250944L, 
    14.73384247358929905561314L,   15.29181976688274097174679L
};

static const long double w128[64] = {
    0.194097611864087756977697L,      0.179773083907799264988698L, 
    0.1542104352983543833635277L,     0.1225032731641356946186646L, 
    0.09010867837644891954805744L,    0.06136072100449006566465107L, 
    0.03867395481063690265502489L,    0.02255431016782442241024982L, 
    0.01216691886446933949101663L,    0.006068862406925887620668014L, 
    0.002797839401605789273190804L,   0.001191563814457167239116806L, 
    0.0004685515378084113654798021L,  0.0001700140882628094094098972L, 
    0.00005688743760040241092701879L, 0.00001754048584809390503836776L, 
    4.979924532590987011340993e-6L,   1.300747003238199233513756e-6L, 
    3.12287298617890308197945e-7L,    6.884581122154350090644063e-8L, 
    1.392190715293517881195788e-8L,   2.579397229426394801149806e-9L, 
    4.373186659848403445632173e-10L,  6.775780487774553786308396e-11L, 
    9.580316508735857708620664e-12L,  1.234214486600556690816236e-12L, 
    1.446347321190416563205909e-13L,  1.539049730353545814249811e-14L, 
    1.484223837513856482911896e-15L,  1.294548159339371534395457e-16L, 
    1.018933230423292524036582e-17L,  7.220107316928292019644377e-19L, 
    4.594007677329721592211726e-20L,  2.617457583934811155868732e-21L, 
    1.331367859033589604405994e-22L,  6.02598403200645428864657e-24L, 
    2.418403459647664969603906e-25L,  8.572830483769353744549325e-27L, 
    2.672923620058073240172664e-28L,  7.296545006768404253818687e-30L, 
    1.735103020282061208816017e-31L,  3.57437889587942107216457e-33L, 
    6.33991352636648906076754e-35L,   9.616708067967506977595218e-37L, 
    1.238085557976368037618838e-38L,  1.341497481764369366965568e-40L, 
    1.211779534130591907354349e-42L,  9.028040138786644009179616e-45L, 
    5.480217028978796498206163e-47L,  2.67274375173606785452022e-49L, 
    1.030486252055694734226723e-51L,  3.082077383339298687104255e-54L, 
    6.993072924051955987987474e-57L,  1.171978501212980517385599e-59L, 
    1.404467257740487260441866e-62L,  1.156155164096375213347254e-65L, 
    6.214244161830313662409307e-69L,  2.041585797243985015800692e-72L, 
    3.751215868804724996562746e-76L,  3.401230086936637126866929e-80L, 
    1.261249483338538303309322e-84L,  1.404689771315088634798657e-89L, 
    2.608172402409111079248851e-95L,  1.799065980109284720823363e-102L
};

static const long double x256[128] = {
    0.06935239452955744388803576L, 0.2080597846328379521498735L, 
    0.3467749791369948124594288L, 0.4855031852056820552722951L, 
    0.6242496163534804348461547L, 0.7630194949989306903749989L, 
    0.9018180550302726614522567L, 1.040650544387582565786614L, 
    1.179522227665047701676857L, 1.31843838873717568219942L, 
    1.457404333412804320782881L, 1.596425392120858802572519L, 
    1.735506922631895184223701L, 1.87465431281957401864391L, 
    2.013872983466325516855802L, 2.153168391117598720543739L, 
    2.292546030989232322177876L, 2.432011439932644768316125L, 
    2.571570199462716932064948L, 2.711227938853432848246467L, 
    2.850990338306553771418865L, 2.990863132198829248693306L, 
    3.130852112413497215693322L, 3.270963131762094667648482L, 
    3.411202107502892705514789L, 3.551575024962586333004501L, 
    3.692087941268212069044807L, 3.832746989196637202265515L, 
    3.973558381149365504012361L, 4.114528413260837801407682L, 
    4.255663469648874594039856L, 4.396970026816414737676318L, 
    4.538454658214252262608834L, 4.680124038975066105806037L, 
    4.821984950829678728288242L, 4.964044287217173463977855L, 
    5.10630905860125163719935L,  5.248786398006024106363446L, 
    5.391483566785313588799576L, 5.534407960640500139418677L, 
    5.677567115902979401907698L, 5.820968716098429376480255L, 
    5.964620598811304935295807L, 6.108530762869309579656447L, 
    6.252707375869041431515258L, 6.397158782065586819727379L, 
    6.541893510650553014329482L, 6.686920284444906124242071L, 
    6.832248029035027029217979L, 6.977885882382635487649597L, 
    7.123843204941680417740606L, 7.270129590317975394266941L, 
    7.416754876510298003102638L, 7.563729157774898358033904L, 
    7.711062797158907892662314L, 7.858766439752040657111136L, 
    8.006851026710276598100182L, 8.155327810109955839254568L, 
    8.304208368695947079898085L, 8.453504624593341194265637L, 
    8.603228861058530323780014L, 8.753393741352639920944896L, 
    8.904012328828173805034043L, 9.055098108328510279622257L, 
    9.206665009009665162900102L, 9.358727428704646441648322L, 
    9.511300259962916033897065L, 9.664398917911120590604498L, 
    9.818039370096555953596296L, 9.972238168492020856184266L, 
    10.12701248386006377403642L, 10.28238014269644526003412L, 
    10.43835966699729105323386L, 10.5949703171223244559366L, 
    10.75223213805823855355158L, 10.91016600942228543161929L, 
    11.06879369958721035788726L, 11.22813792435555867835981L, 
    11.38822241066509722915626L, 11.54907196586876769074542L, 
    11.71071255320359456849201L, 11.87317137414494220608773L, 
    12.03647695843741873836146L, 12.20065926270392853159678L, 
    12.36574977866274341595387L, 12.53178165213247638636606L, 
    12.69878981418074280983294L, 12.86681112597928088568419L, 
    13.03588453917276977741826L, 13.2060512738584353781359L, 
    13.37735501661859040604692L, 13.54984214146078065446075L, 
    13.72356195701559996039873L, 13.89856698393993565069987L, 
    14.07491326719811317981342L, 14.25266072877669929313429L, 
    14.43187356747123445644847L, 14.61262071371756574751318L, 
    14.79497634909551278489501L, 14.97902050219883259813004L, 
    15.16483973516297914368827L, 15.3525279384318336578543L, 
    15.5421872555438623820107L,  15.73392916512399133977845L, 
    15.9278757542898247417255L,  16.12416122689225220690767L, 
    16.3229337022207934073675L,  16.52435737617411960358499L, 
    16.72861513911482577927869L, 16.93591177519067212055642L, 
    17.14647791056563829576116L, 17.36057493851576722552809L, 
    17.57850123670882529979305L, 17.80060012061807779309298L, 
    18.02727017059946945879922L, 18.2589788687186970374216L, 
    18.49628095483317150353413L, 18.73984368625242609717427L, 
    18.99048250044257345197167L, 19.24921290925104953283575L, 
    19.51732878957217145380639L, 19.79652581044909244607022L, 
    20.08910699282410079594748L, 20.39835005676927165192042L, 
    20.72922879233067683193546L, 21.09003212218722581397947L, 
    21.49683653764692225297995L, 21.99169337968173143150578L
};

static const long double w256[128] = {
    0.1380396862755209115461623L,  0.1328339180744035877840838L, 
    0.1230032311351158193938309L,  0.1096027801343023138515433L, 
    0.09397592780705028198843521L, 0.07753390349069548171389453L, 
    0.06155085702479888137715045L, 0.04701418448130623325869308L, 
    0.03455082586657174270549896L, 0.02442882040773177575688892L, 
    0.01661645356548084220065692L,  0.0108727800108988110568655L, 
    0.006843547910376821683595415L,  0.004143142376403676039203141L, 
    0.002412420213191878584310166L,  0.001350873369460276276263061L, 
    0.0007274048672835041690488583L, 0.0003766137634591824695368737L, 
    0.0001874693790569979452285423L, 0.00008970808308135376355184578L, 
    0.00004126196754634871562674561L, 0.00001824032324514029342130518L, 
    7.748604996077851148280404e-6L,  3.162744043403113931818247e-6L, 
    1.24020126909208086348216e-6L,   4.671354640886792791901543e-7L, 
    1.689848640543874893179345e-7L,  5.869965921453493544025045e-8L, 
    1.957633323005871267670621e-8L,  6.266959900264999864392682e-9L, 
    1.925442790705171732963742e-9L,  5.676325033901026938569875e-10L, 
    1.60538054513839834375098e-10L,  4.354820861532416579395074e-11L, 
    1.132785812294511238181697e-11L, 2.824946689715659682286396e-12L, 
    6.752325874818933232891002e-13L, 1.546567308104700068751173e-13L, 
    3.39346935552010561490731e-14L,  7.131185523906222566740663e-15L, 
    1.434831081080417221792584e-15L, 2.763344140579202199681171e-16L, 
    5.092528763567802528354142e-17L, 8.977606649060171674352775e-18L, 
    1.513475516755380230480198e-18L, 2.43911295290580876119099e-19L, 
    3.756458877230342905585748e-20L, 5.526606033164875076303145e-21L, 
    7.764413344115745904428389e-22L, 1.041262400983358880656992e-22L, 
    1.332410438727469877702362e-23L, 1.62615002782606333379254e-24L, 
    1.892080808799356026540107e-25L, 2.097876957276834807919888e-26L, 
    2.21553769695818324398318e-27L,  2.227548036903074988264866e-28L, 
    2.13111054341846815130745e-29L,  1.939053677322792162950919e-30L, 
    1.677042649664920815789748e-31L, 1.377924367367343102852255e-32L, 
    1.074931862337096572263033e-33L, 7.956981568447057548598483e-35L, 
    5.585409873344557681057222e-36L, 3.715504664140972243259673e-37L, 
    2.340681275797534661401565e-38L, 1.395476131078660086257215e-39L, 
    7.867556834480307350178847e-41L, 4.191442393815232424216786e-42L, 
    2.108378037437585132328501e-43L, 1.000543082941735644142381e-44L, 
    4.475600887692284974447282e-46L, 1.885407410437632771439863e-47L, 
    7.472925820097877636516094e-49L, 2.784090389779379680086744e-50L, 
    9.739575926913210786089395e-52L, 3.195934195189601973735105e-53L, 
    9.825906543715608806294447e-55L, 2.827209869386944581508349e-56L, 
    7.603681463947765852817714e-58L, 1.90903879280922896335848e-59L, 
    4.468359563130094723099765e-61L, 9.736721448687695449179484e-63L, 
    1.97227114458311327184445e-64L,  3.707957510681315723932626e-66L, 
    6.459617468090556176866071e-68L, 1.04096106973688862183115e-69L, 
    1.548918934901887097068285e-71L, 2.124008592959635402413398e-73L, 
    2.678777087634353537096247e-75L, 3.100541476868721130506631e-77L, 
    3.286028981833529405445274e-79L, 3.181191265208711677863583e-81L, 
    2.805940677661302029905023e-83L, 2.24880261652561015773325e-85L, 
    1.632839465032475722441423e-87L, 1.070788688575716803356984e-89L, 
    6.321004519105135997710051e-92L, 3.346876186519417166548869e-94L, 
    1.583425765728327873987018e-96L, 6.66599405444759393471914e-99L, 
    2.486008353731566366464935e-101L, 8.173567963136183165784288e-104L, 
    2.356748211065233246206346e-106L, 5.92550121227430141957195e-109L, 
    1.29102717331080240316334e-111L,  2.420862690260208600845504e-114L, 
    3.877534426033000916320577e-117L, 5.26105468280227923865402e-120L, 
    5.991040366178925403442063e-123L, 5.667053365630981877133338e-126L, 
    4.401509171610101608985115e-129L, 2.770425736852218227029787e-132L, 
    1.392265195551478902820944e-135L, 5.491764305469765268892198e-139L, 
    1.667020806306620136345465e-142L, 3.805347862683777480481036e-146L, 
    6.356833254660565102135512e-150L, 7.521718497069817935532254e-154L, 
    6.059009685126994742423406e-158L, 3.163462321390488738943512e-162L, 
    1.005951800115730053476254e-166L, 1.796596257267014451909506e-171L, 
    1.615495504989693244106132e-176L, 6.267289952280966854143155e-182L, 
    8.315216118625034527572313e-188L, 2.571712580908051776342705e-194L, 
    8.899167767891703150223339e-202L, 5.235854530678407140045257e-211L
};

namespace npstat {
    GaussHermiteQuadrature::GaussHermiteQuadrature(const unsigned npoints)
        : a_(0),
          w_(0),
          npoints_(npoints)
    {
        switch (npoints)
        {
        case 16U:
            a_ = x16;
            w_ = w16;
            break;

        case 32U:
            a_ = x32;
            w_ = w32;
            break;

        case 64U:
            a_ = x64;
            w_ = w64;
            break;

        case 100U:
            a_ = x100;
            w_ = w100;
            break;

        case 128U:
            a_ = x128;
            w_ = w128;
            break;

        case 256U:
            a_ = x256;
            w_ = w256;
            break;

        default:
            npoints_ = 0;
            throw std::invalid_argument(
                "In npstat::GaussHermiteQuadrature constructor: "
                "unsupported number of abscissae requested");
        }
    }

    std::vector<unsigned> GaussHermiteQuadrature::allowedNPonts()
    {
        std::vector<unsigned> v(allowed, allowed+sizeof(allowed)/sizeof(allowed[0]));
        return v;
    }

    unsigned GaussHermiteQuadrature::minimalExactRule(const unsigned polyDegree)
    {
        for (unsigned i=0; i<sizeof(allowed)/sizeof(allowed[0]); ++i)
            if (polyDegree < 2U*allowed[i])
                return allowed[i];
        return 0U;
    }

    bool GaussHermiteQuadrature::isAllowed(const unsigned npoints)
    {
        for (unsigned i=0; i<sizeof(allowed)/sizeof(allowed[0]); ++i)
            if (npoints == allowed[i])
                return true;
        return false;
    }

    void GaussHermiteQuadrature::getAbscissae(
        long double* abscissae, const unsigned len) const
    {
        const unsigned halfpoints = npoints_/2;
        if (len < halfpoints) throw std::invalid_argument(
            "In npstat::GaussHermiteQuadrature::getAbscissae: "
            "unsifficient length of the output buffer");
        assert(abscissae);
        assert(a_);
        for (unsigned i=0; i<halfpoints; ++i)
            abscissae[i] = a_[i];
    }

    void GaussHermiteQuadrature::getWeights(
        long double* weights, const unsigned len) const
    {
        const unsigned halfpoints = npoints_/2;
        if (len < halfpoints) throw std::invalid_argument(
            "In npstat::GaussHermiteQuadrature::getWeights: "
            "unsifficient length of the output buffer");
        assert(weights);
        assert(w_);
        for (unsigned i=0; i<halfpoints; ++i)
            weights[i] = w_[i];
    }
}